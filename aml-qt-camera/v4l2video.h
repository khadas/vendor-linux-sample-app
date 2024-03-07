#ifndef V4L2VIDEO_H
#define V4L2VIDEO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <linux/videodev2.h>
#include <stdint.h>
#include <asm/types.h>
#include <iostream>
#include <map>
#include <QString>
#include <QComboBox>

#define	DEFAULT_DEVICE_NAME		"/dev/video0"

class V4l2Video
{
private:
    typedef struct VideoBuffer
    {
        unsigned char 	*pStart;
        unsigned int 	uLength;
    } VideoBuffer;

    VideoBuffer		*pBuffers;
    int				fd;
    unsigned int 	uCount = 4;
    int				uIndex;

    typedef struct Format
    {
        __u32 width;
        __u32 height;
        __u32 pixel_format;
    } Format;
    Format default_fmt;

    int64_t pts;

public:
    V4l2Video();
    ~V4l2Video();

    int openDevice(const char* device_name);
    int closeDevice(int fd);

    int isValidDevice(int fd);
    int setCaptureFormat(int fd, struct v4l2_format *fmt);
    int setCaptureParam(int fd, struct v4l2_streamparm *stream_param);
    QString getDefaultResolution();

    int getSupportedFormat(int fd, QString &format_text,
                QMap<__u32, QMap<QString, QVector<QString> > > &camera_format);
    int initDevice(int fd, QString &format_text,
                QMap<__u32, QMap<QString, QVector<QString> > > &camera_format);
    int initBuffer(int fd);

    int startCapturing(int fd);
    int stopCapturing(int fd);

    int checkVideoFd(int fd);
    int dequeueFrame(int fd, void **out_frame, int *frame_len, int *index);
    int queueFrame(int fd, int index);
};

#endif // V4L2VIDEO_H
