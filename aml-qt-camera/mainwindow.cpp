#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "v4l2video.h"
#include <QDebug>
#include <QCameraInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QSignalMapper>
#include <QMap>
#include <iostream>
#include<string>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ui->stop_camera->setDisabled(true);
    this->ui->start_camera->setCheckable(true);
    this->ui->stop_camera->setCheckable(true);

    this->ui->camera_group->addButton(this->ui->start_camera);
    this->ui->camera_group->addButton(this->ui->stop_camera);
    this->ui->camera_group->setExclusive(true);

    this->m_v4l2video = new V4l2Video;

    this->ui->label->setText("");
    this->ui->label->setStyleSheet("QLabel{background-color:rgb(169,169,169);}");


    QMap<QString, int> device_list;
    int ret = getVideoDeviceList(device_list);

    int count = 0;
    QString format_text;
    foreach(const QString &str, device_list.keys())
    {
        this->ui->device_name_list->addItem(str, device_list.value(str));

        if (str.compare(DEFAULT_DEVICE_NAME) == 0) {
            this->work_thread = new ThreadWork(this->m_v4l2video, device_list.value(str));
            connect(work_thread, SIGNAL(UpdateFrame(QImage *)), this, SLOT(update_frame(QImage *)));
            connect(work_thread, SIGNAL(UpdateBackground()), this, SLOT(update_background()));
            connect(this, SIGNAL(StartRunning()), work_thread, SLOT(start_running()));
            connect(this, SIGNAL(StopRunning()), work_thread, SLOT(stop_running()));

            if ( -1 == this->m_v4l2video->initDevice(device_list.value(str), format_text, this->camera_format) /* ||
                -1 == this->m_v4l2video->initBuffer(device_list.value(str))*/)
            {
                this->m_v4l2video->closeDevice(device_list.value(str));
            }
            this->ui->textBrowser->setText(format_text);
        }
        count++;
    }

    // QCombox: show default resolution and related fps
    QMap<__u32, QMap<QString, QVector<QString> > >::Iterator iter;
    QString default_resolution = this->m_v4l2video->getDefaultResolution();
    for (iter = this->camera_format.begin(); iter != this->camera_format.end(); iter++) {
        if (iter.key() == V4L2_PIX_FMT_MJPEG) {
            this->ui->resolution->clear();
            QMap<QString, QVector<QString> > reso_fps = iter.value();
            QMap<QString, QVector<QString> >::Iterator it;
            for (it = reso_fps.begin(); it != reso_fps.end(); it++) {
                this->ui->resolution->addItem(it.key());
                if (QString::compare(QString(it.key()), default_resolution) == 0) {
                    // set current value: default resolution
                    this->ui->resolution->setCurrentText(default_resolution);
                    this->ui->fps->clear();
                    QVector<QString> fps = it.value();
                    for (int i = 0; i < fps.size(); i++) {
                        this->ui->fps->addItem(fps.at(i));
                    }
                }
            }
        }
    }

}

MainWindow::~MainWindow()
{
    disconnect(work_thread);
    disconnect(this);
    int fd = this->ui->device_name_list->currentData().toInt();
    if (fd > 0)
    {
        this->m_v4l2video->closeDevice(fd);
    }
    delete ui;
}

int MainWindow::getVideoDeviceList(QMap<QString, int> &device_list)
{
    device_list.clear();

    const char* dir_name = "/dev";
    DIR* dir = opendir(dir_name);
    if (!dir)
    {
        fprintf(stderr, "open the directory: %s failed\n", dir_name);
        return -1;
    }

    struct dirent* entry = nullptr;
    int fd;

    while ((entry = readdir(dir)))
    {
        char device_name[128];
        if (strncmp(entry->d_name, "video", 5))
            continue;

        snprintf(device_name, sizeof(device_name), "/dev/%s", entry->d_name);

        if ((fd = m_v4l2video->openDevice(device_name)) < 0)
            continue;

        if (m_v4l2video->isValidDevice(fd) != 0)
            continue;

        device_list.insert(device_name, fd);
#if 1
        if (strcmp(device_name, "/dev/video0") != 0)
        {
            m_v4l2video->closeDevice(fd);
        }
#endif
        continue;
    }

    closedir(dir);
    return 0;
}

void MainWindow::on_device_name_list_currentIndexChanged(const QString &arg1)
{
   /*
    * IF NEEDED:
    * Video Frame may come from different video device (/dev/video*)
    * This slot function could be triggered
    */
   qDebug() << "current device:" << arg1 << "  fd:"
            << this->ui->device_name_list->currentData().toInt() << "\n";
}

void MainWindow::update_frame(QImage *image)
{
    //Image.scaled(this->ui->label->size(), Qt::KeepAspectRatio);
    this->ui->label->setPixmap(QPixmap::fromImage(*image));
    //this->ui->label->setScaledContents(true);
    this->ui->label->setAlignment(Qt::AlignCenter);
    delete image;
}

void MainWindow::update_background()
{
    qDebug() << "update back ground" << "\n";
    this->ui->label->setStyleSheet("QLabel{background-color:rgb(169,169,169);}");   // grey
    this->ui->label->setPixmap(QPixmap());
}

void MainWindow::on_start_camera_clicked()
{
    qDebug() << "Start Camera" << "\n";
    this->ui->start_camera->setDisabled(true);
    this->ui->stop_camera->setEnabled(true);
    int fd = this->ui->device_name_list->currentData().toInt();

    if (-1 == this->m_v4l2video->initBuffer(fd))
    {
        return;
    }

    if (this->ui->device_name_list->currentText().compare("/dev/video0") != 0)
    {
        qDebug() << "Only support video0 device" << "\n";
        return;
    }

    int ret = this->m_v4l2video->startCapturing(fd);
    if (-1 == ret)
    {
        this->m_v4l2video->stopCapturing(fd);
        this->m_v4l2video->closeDevice(fd);
        return;
    }

    // start work thread
    this->work_thread->start();
    emit StartRunning();
}

void MainWindow::on_stop_camera_clicked()
{
    this->ui->stop_camera->setDisabled(true);
    this->ui->start_camera->setEnabled(true);

    if (this->ui->device_name_list->currentText().compare("/dev/video0") != 0)
    {
        qDebug() << "Only support video0 device" << "\n";
        return;
    }

    // stop work thread
    emit StopRunning();
    this->work_thread->quit();
    this->work_thread->wait();

    int fd = this->ui->device_name_list->currentData().toInt();
    this->m_v4l2video->stopCapturing(fd);
}


void MainWindow::on_resolution_currentTextChanged(const QString &arg1)
{
    qDebug() << "changed resolution: " << arg1 << "\n";

    QMap<__u32, QMap<QString, QVector<QString> > >::Iterator iter;
    for (iter = this->camera_format.begin(); iter != this->camera_format.end(); iter++) {
        if (iter.key() == V4L2_PIX_FMT_MJPEG) {
            QMap<QString, QVector<QString> > reso_fps = iter.value();
            QMap<QString, QVector<QString> >::Iterator it;
            for (it = reso_fps.begin(); it != reso_fps.end(); it++) {
                if (QString::compare(QString(it.key()), arg1) == 0) {
                    this->ui->fps->clear();
                    QVector<QString> fps = it.value();
                    for (int i = 0; i < fps.size(); i++) {
                        this->ui->fps->addItem(fps.at(i));
                    }
                }
            }
        }
    }

    QStringList stringlist = arg1.split("x");

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.width = QString(stringlist.at(0)).toInt();
    fmt.fmt.pix.height = QString(stringlist.at(1)).toInt();
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    int fd = this->ui->device_name_list->currentData().toInt();
    this->m_v4l2video->setCaptureFormat(fd, &fmt);

}


void MainWindow::on_fps_currentTextChanged(const QString &arg1)
{
    int fd = this->ui->device_name_list->currentData().toInt();
    int fps = arg1.toInt();

    struct v4l2_streamparm stream_param;
    stream_param.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_param.parm.capture.timeperframe.numerator = 1;
    stream_param.parm.capture.timeperframe.denominator = fps;
    this->m_v4l2video->setCaptureParam(fd, &stream_param);
}
