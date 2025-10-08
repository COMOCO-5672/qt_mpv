#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QTimer>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mpvWidget = new MpvWidget(this);
    setCentralWidget(mpvWidget);
    
    setWindowTitle("Qt MPV Player");
    //resize(800, 600);
    
    // 自动播放视频
    QTimer::singleShot(100, this, [this]() {
        // 自动查找视频文件
        QString videoDir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        QDir dir(videoDir);
        QStringList videos = dir.entryList({"*.mp4", "*.mkv", "*.avi", "*.mov", "*.mpg"}, QDir::Files);
        
        if (!videos.isEmpty()) {
            QString file = dir.filePath(videos.first());
            mpvWidget->loadFile(file);
            mpvWidget->play();
            showFullScreen();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
