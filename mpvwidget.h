#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget();

    void loadFile(const QString &file);
    void play();
    void pause();
    void stop();
    void seek(int seconds);
    void setVolume(int volume);
    bool isPlaying() const;
    double duration() const;
    double position() const;

signals:
    void durationChanged(double duration);
    void positionChanged(double position);
    void stateChanged(bool playing);

protected:
    void initializeGL() override;
    void paintGL() override;

private slots:
    void onFrameSwapped();

private:
    static void on_mpv_events(void *ctx);
    void handleMpvEvents();
    
    mpv_handle *mpv_;
    mpv_render_context *mpv_gl_context_;
    bool is_playing_ = false;
    double duration_ = 0;
    double position_ = 0;
};

#endif // MPVWIDGET_H