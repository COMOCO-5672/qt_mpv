#include "mpvwidget.h"
#include <QOpenGLContext>
#include <QDebug>
#include <QTimer>

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glContext = QOpenGLContext::currentContext();
    return glContext ? (void *)glContext->getProcAddress(name) : nullptr;
}

MpvWidget::MpvWidget(QWidget *parent) : QOpenGLWidget(parent) {
    mpv_ = mpv_create();
    if (!mpv_) {
        qFatal("Could not create mpv context");
    }
    
    // 设置 mpv 选项
    mpv_set_option_string(mpv_, "config", "no");
    mpv_set_option_string(mpv_, "input-default-bindings", "yes");
    mpv_set_option_string(mpv_, "input-vo-keyboard", "yes");
    mpv_set_option_string(mpv_, "osc", "no");
    
    // Windows 特定设置
#ifdef _WIN32
    mpv_set_option_string(mpv_, "gpu-api", "d3d11");
    mpv_set_option_string(mpv_, "hwdec", "d3d11va");
#endif
    
    if (mpv_initialize(mpv_) < 0) {
        qFatal("Could not initialize mpv context");
    }
    
    // 设置事件回调
    mpv_set_wakeup_callback(mpv_, &MpvWidget::on_mpv_events, this);
}

MpvWidget::~MpvWidget() {
    if (mpv_gl_context_) {
        mpv_render_context_free(mpv_gl_context_);
    }
    if (mpv_) {
        mpv_terminate_destroy(mpv_);
    }
}

void MpvWidget::initializeGL() {
    // 创建 mpv 的 OpenGL 渲染上下文
    mpv_opengl_init_params gl_init{get_proc_address, nullptr};
    
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    
    if (mpv_render_context_create(&mpv_gl_context_, mpv_, params) < 0) {
        qFatal("Failed to create mpv render context");
    }
    
    // 连接 frameSwapped 信号
    connect(this, &QOpenGLWidget::frameSwapped, this, &MpvWidget::onFrameSwapped);
}

void MpvWidget::paintGL() {
    if (!mpv_gl_context_) return;
    
    mpv_opengl_fbo fbo{
        .fbo = 0,
        .w = width(),
        .h = height(),
        .internal_format = 0
    };
    
    int flip_y = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    
    mpv_render_context_render(mpv_gl_context_, params);
}

void MpvWidget::onFrameSwapped() {
    // 触发重绘
    update();
}

void MpvWidget::loadFile(const QString &path) {
    const QByteArray path_utf8 = path.toUtf8();
    const char *cmd[] = {"loadfile", path_utf8.constData(), NULL};
    mpv_command_async(mpv_, 0, cmd);
}

void MpvWidget::play() {
    const char *cmd[] = {"set", "pause", "no", NULL};
    mpv_command_async(mpv_, 0, cmd);
    is_playing_ = true;
    emit stateChanged(true);
}

void MpvWidget::pause() {
    const char *cmd[] = {"set", "pause", "yes", NULL};
    mpv_command_async(mpv_, 0, cmd);
    is_playing_ = false;
    emit stateChanged(false);
}

void MpvWidget::stop() {
    const char *cmd[] = {"stop", NULL};
    mpv_command_async(mpv_, 0, cmd);
    is_playing_ = false;
    emit stateChanged(false);
}

void MpvWidget::seek(int seconds) {
    const char *cmd[] = {"seek", QString::number(seconds).toUtf8().constData(), "relative", NULL};
    mpv_command_async(mpv_, 0, cmd);
}

void MpvWidget::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    const char *cmd[] = {"set", "volume", QString::number(volume).toUtf8().constData(), NULL};
    mpv_command_async(mpv_, 0, cmd);
}

bool MpvWidget::isPlaying() const {
    return is_playing_;
}

double MpvWidget::duration() const {
    return duration_;
}

double MpvWidget::position() const {
    return position_;
}

// 静态事件回调
void MpvWidget::on_mpv_events(void *ctx) {
    QMetaObject::invokeMethod(static_cast<MpvWidget*>(ctx), "handleMpvEvents", Qt::QueuedConnection);
}

// 处理事件队列
void MpvWidget::handleMpvEvents() {
    while (mpv_) {
        mpv_event *event = mpv_wait_event(mpv_, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        
        // 处理事件
        switch (event->event_id) {
        case MPV_EVENT_FILE_LOADED:
            qDebug() << "File loaded";
            // 获取视频时长
            mpv_get_property_async(mpv_, 0, "duration", MPV_FORMAT_DOUBLE);
            break;
            
        case MPV_EVENT_END_FILE:
            qDebug() << "Playback ended";
            is_playing_ = false;
            emit stateChanged(false);
            break;
            
        case MPV_EVENT_PROPERTY_CHANGE: {
            mpv_event_property *prop = (mpv_event_property *)event->data;
            if (strcmp(prop->name, "duration") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                duration_ = *(double *)prop->data;
                emit durationChanged(duration_);
            }
            else if (strcmp(prop->name, "time-pos") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                position_ = *(double *)prop->data;
                emit positionChanged(position_);
            }
            else if (strcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG) {
                is_playing_ = !*(int *)prop->data;
                emit stateChanged(is_playing_);
            }
            break;
        }
            
        case MPV_EVENT_LOG_MESSAGE: {
            mpv_event_log_message *msg = (mpv_event_log_message *)event->data;
            qDebug() << "MPV LOG:" << msg->prefix << msg->level << msg->text;
            break;
        }
        }
    }
}