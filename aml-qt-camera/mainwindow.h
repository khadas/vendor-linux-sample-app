#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include "v4l2video.h"
#include <QtGui>
#include <QtWidgets/QLabel>
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QMutex>
#include "threadwork.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;

    V4l2Video       *m_v4l2video;
    QLabel			*pLabel;
    unsigned char	*pVideoData;
    int				uDataLength;
    int             default_device_fd;
    ThreadWork      *work_thread;
    QMap<__u32, QMap<QString, QVector<QString> > > camera_format;

    int getVideoDeviceList(QMap<QString, int> &device_list);

signals:
    void StartRunning();
    void StopRunning();
    void QueueBuffer();


private slots:
    void update_frame(QImage *);
    void update_background();

    void on_device_name_list_currentIndexChanged(const QString &arg1);
    void on_start_camera_clicked();
    void on_stop_camera_clicked();
    void on_resolution_currentTextChanged(const QString &arg1);
    void on_fps_currentTextChanged(const QString &arg1);
};
#endif // MAINWINDOW_H
