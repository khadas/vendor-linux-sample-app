#include "threadwork.h"
#include <QImage>

ThreadWork::ThreadWork(V4l2Video *v4l2, int fd)
{
    this->v_param = v4l2;
    this->dev_fd = fd;
    this->running = false;
}

ThreadWork::~ThreadWork()
{

}

void ThreadWork::run()
{
    qDebug() << "In my work thread, id:" << QThread::currentThreadId()
             << ", dev_fd:" << dev_fd << "\n";
    unsigned char	*pVideoData;
    int				uDataLength;
    int             uIndex;
    QImage *image;

    while (running)
    {
        if (v_param->checkVideoFd(dev_fd) == 0)
        {
            v_param->dequeueFrame(dev_fd, (void **)(&(pVideoData)), &uDataLength, &uIndex);
            image = new QImage();
            image->loadFromData(pVideoData, uDataLength);

            emit UpdateFrame(image);
            v_param->queueFrame(dev_fd, uIndex);
        }
    }
    emit UpdateBackground();
}

void ThreadWork::start_running()
{
    running = true;
}

void ThreadWork::stop_running()
{
    running = false;
}

