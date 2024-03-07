#ifndef THREADWORK_H
#define THREADWORK_H

#include <QThread>
#include <QDebug>
#include <QMutex>
#include "v4l2video.h"

class ThreadWork : public QThread
{
    Q_OBJECT

public:
    ThreadWork(V4l2Video *v4l2, int fd);
    ~ThreadWork();

private:
    V4l2Video       *v_param;
    int             dev_fd;
    bool            running;


signals:
    void UpdateFrame(QImage *);
    void UpdateBackground();

protected:
    void run() override;

private slots:
    void start_running();
    void stop_running();
};


#endif // THREADWORK_H
