#include "video.h"
#include "mainwindow.h"
#include "ui_video.h"

video::video(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::video)
{
    ui->setupUi(this);
    this->setWindowTitle("Video");
    this->setFixedSize(800,480);

    mplayerProcess = new QProcess(this);

    // 初始化默认音乐路径
    defaultVideoPath = "/video";
    currentFilePath = "";
    currentIndex = -1;

    // 获取所有视频文件
    loadMusicFiles();

    connect(mplayerProcess, &QProcess::readyReadStandardOutput, this, &video::handleMPlayerOutput);
    connect(mplayerProcess, &QProcess::readyReadStandardError, this, &video::handleMPlayerError);

    // 初始化播放按钮
    ui->start_pause->setIcon(QIcon(":/images/pause.svg"));
    ui->start_pause->setIconSize(QSize(48, 48));

}


video::~video()
{
    delete ui;
}


void video::loadMusicFiles()
{
    QStringList filters;
    filters << "*.mp4";
    QDir dir(defaultVideoPath);
    dir.setNameFilters(filters);
    videoFiles = dir.entryList(QDir::Files);
    for (int i = 0; i < videoFiles.size(); ++i) {
        videoFiles[i] = dir.absoluteFilePath(videoFiles.at(i));
    }
}

void video::handleMPlayerOutput()
{
    while (mplayerProcess->canReadLine()) {
        QString message(mplayerProcess->readLine());
        qDebug() << message << endl;
        QStringList message_list = message.split("=");
    }
}

void video::handleMPlayerError()
{
    QByteArray error = mplayerProcess->readAllStandardError();
    printf("MPlayer Error: %s\r\n", error.constData());
}

void video::mousePressEvent(QMouseEvent *event)
{
    printf("mousePressEvent\r\n");
    if (event->button() == Qt::LeftButton) {

    }
    QMainWindow::mousePressEvent(event); // 保持默认行为
}

void video::playVideo(const QString &filePath)
{
    if (mplayerProcess->state() == QProcess::Running) {
        mplayerProcess->terminate(); // 终止当前正在运行的 mplayer 进程
        if (!mplayerProcess->waitForFinished(3000)) { // 等待进程终止，超时3秒
            mplayerProcess->kill();
        }
    }

    // 获取 video_widget 的窗口 ID
    WId windowID = ui->video_widget->winId();
    qDebug() << "video_widget window ID:" << windowID;

    // 设置 MPlayer 的参数，包含 -wid 选项
    QStringList args;
    args << "-slave" << "-quiet" << "-wid" << QString::number(windowID) << filePath << "-vo" << "fbdev2";
    qDebug() << "MPlayer arguments:" << args;

    mplayerProcess->start("mplayer", args);

    if (!mplayerProcess->waitForStarted()) {
        printf("mplayer error\r\n");
        return;
    }

    play_flag = true;
    ui->start_pause->setIcon(QIcon(":/images/start.svg"));

    // 获取视频名称并设置到 QLabel
    QString videoName = QFileInfo(filePath).completeBaseName();
    ui->label_video_name->setText(videoName);
}

void video::on_start_pause_clicked()
{
    printf("on_start_pause_clicked\r\n");

#ifdef __arm__
    if (mplayerProcess->state() != QProcess::Running) {
        if (videoFiles.isEmpty()) {
            QMessageBox::warning(this, "No Video", "No video files found in the default directory.");
            return;
        }

        int randomIndex = QRandomGenerator::global()->bounded(videoFiles.size());
        currentFilePath = videoFiles.at(randomIndex);
        currentIndex = randomIndex;

        playVideo(currentFilePath);
    } else {
        mplayerProcess->write("pause\n");
        if(play_flag == true){
            ui->start_pause->setIcon(QIcon(":/images/pause.svg"));
            ui->start_pause->setIconSize(QSize(48, 48));
            play_flag = false;
        }
        else if(play_flag == false){
            ui->start_pause->setIcon(QIcon(":/images/start.svg"));
            ui->start_pause->setIconSize(QSize(48, 48));
            play_flag = true;
        }
    }
#else
    if(play_flag == true){
        ui->start_pause->setIcon(QIcon(":/images/pause.svg"));
        ui->start_pause->setIconSize(QSize(48, 48));
        play_flag = false;
    }
    else if(play_flag == false){
        ui->start_pause->setIcon(QIcon(":/images/start.svg"));
        ui->start_pause->setIconSize(QSize(48, 48));
        play_flag = true;
    }
#endif
}

void video::on_video_back_clicked()
{
    printf("on_video_back_clicked\r\n");

    if (mplayerProcess->state() == QProcess::Running) {
        mplayerProcess->write("quit\n");
        mplayerProcess->waitForBytesWritten(1000);
        mplayerProcess->waitForFinished(3000);  // 等待mplayer进程结束
    }

    this->close();

    MainWindow *m = new MainWindow();
    m->show();
}