#include "v4l2video.h"
#include <QDebug>
#include <iostream>
#include <sys/time.h>

V4l2Video::V4l2Video()
{
    this->pBuffers = NULL;
    this->fd = -1;
    this->uCount = 5;
    this->uIndex = -1;
}

V4l2Video::~V4l2Video()
{

}

int V4l2Video::openDevice(const char* device_name)
{
    qDebug() << "device_name: " << device_name << "\n";
    int fd = open(device_name, O_RDWR, 0);
    if (-1 == fd)
    {
        perror("open error");
        return	-1;
    }
    qDebug() << "Open device successfully" << "\n";

    return	fd;
}

int V4l2Video::closeDevice(int fd)
{
    if (-1 == close(fd))
    {
        return	-1;
    }

    return	0;
}


int V4l2Video::isValidDevice(int fd)
{
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));

    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        perror("ioctl VIDIOC_QUERYCAP error");
        return	-1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        return	-1;
    }

    if (! (cap.capabilities & V4L2_CAP_STREAMING))
    {
        return	-1;
    }

    printf("Capability infrmations:\n");
    printf("diver : %s \n", cap.driver);
    printf("card : %s \n", cap.card);
    printf("bus_info : %s \n", cap.bus_info);
    printf("version : %08x \n", cap.version);
    printf("capabilities : %08x\n", cap.capabilities);
    printf("\n\n");

    return 0;
}

int V4l2Video::setCaptureFormat(int fd, struct v4l2_format *fmt)
{
    if (-1 == ioctl(fd, VIDIOC_S_FMT, fmt))
    {
        perror("ioctl VIDIOC_S_FMT error");
        return	-1;
    }
    return 0;
}

int V4l2Video::setCaptureParam(int fd, struct v4l2_streamparm *stream_param)
{
    if (-1 == ioctl(fd, VIDIOC_S_PARM, stream_param))
    {
        perror("ioctl VIDIOC_S_PARAM error");
        return	-1;
    }
    return 0;
}

QString V4l2Video::getDefaultResolution()
{
    QString default_resolution = QString::number(default_fmt.width) + "x" + QString::number(default_fmt.height);
    return default_resolution;
}

int V4l2Video::getSupportedFormat(int fd, QString &format_text,
            QMap<__u32, QMap<QString, QVector<QString> > > &camera_format)
{
    // VIDIOC_ENUM_FMT

    int ret = 0;
    struct v4l2_fmtdesc fmt;
    QString str;
    QMap<QString, QVector<QString> > reso_fps;
    QString internal_resolution;
    QVector <QString> fps;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (int i = 0; ; i++)                      // maybe more than one format
    {
        fmt.index = i;
        ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt);
        if (-1 == ret)                      // all kinds of format
        {
            break;
        }

        /* print current image format */
        printf("========== Picture Format: %s ==========\n", fmt.description);
        //str = QString("========== Picture Format: %1 ==========\n").arg(fmt.description);
        str.sprintf("========== Picture Format: %s ==========\n", fmt.description);
        format_text += str;

        /* query related resolution(maybe more than one resolution) depends on current pixel format */
        struct v4l2_frmsizeenum frmsize;
        frmsize.pixel_format = fmt.pixelformat;
        for (int j = 0; ; j++)                  // all kinds of resolution
        {
            frmsize.index = j;
            ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize);
            if (-1 == ret)                  // done
            {
                break;
            }

            /* print current resolution */
            printf(">>>>>>>> resolution: %dx%d <<<<<<<<\n",
                    frmsize.discrete.width, frmsize.discrete.height);
            str = QString(">>>>>>>> resolution: %1x%2 <<<<<<<<\n").arg(frmsize.discrete.width).arg(frmsize.discrete.height);
            internal_resolution = QString("%1x%2").arg(frmsize.discrete.width).arg(frmsize.discrete.height);
            format_text += str;

            // index(0) set to be the current default fmt
            if (V4L2_PIX_FMT_MJPEG == fmt.pixelformat && frmsize.index == 0) {
                this->default_fmt.pixel_format = fmt.pixelformat;
                this->default_fmt.width = frmsize.discrete.width;
                this->default_fmt.height = frmsize.discrete.height;
            }

            struct v4l2_frmivalenum  framival;
            for (int k = 0; ; k++)
            {
                framival.index = k;
                framival.pixel_format = fmt.pixelformat;
                framival.width = frmsize.discrete.width;
                framival.height = frmsize.discrete.height;
                if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &framival) < 0)
                {
                    printf("\n");
                    format_text += "\n";
                    break;
                }

                /* print current fps */
                __u32 fps_tmp = framival.discrete.denominator / framival.discrete.numerator;
                printf("fps:%d ", fps_tmp);
                str = "fps:" + QString::number(fps_tmp);
                format_text += str;
                fps.append(QString::number(fps_tmp));
            }

            printf("fps.size: %d\n", fps.size());
            reso_fps.insert(internal_resolution, fps);
            fps.clear();
        }

        camera_format.insert(fmt.pixelformat, reso_fps);
        reso_fps.clear();
        printf("============== Current Format End ==============\n\n");
        format_text += "============== Current Format End ==============\n\n";

    }
    return 0;
}

int V4l2Video::initDevice(int fd, QString &format_text,
                QMap<__u32, QMap<QString, QVector<QString> > > &camera_format)
{
    if (isValidDevice(fd) != 0)
    {
        qDebug() << "invalid device" << "\n";
        return -1;
    }

    getSupportedFormat(fd, format_text, camera_format);

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.pixelformat = this->default_fmt.pixel_format;
    fmt.fmt.pix.width = this->default_fmt.width;
    fmt.fmt.pix.height = this->default_fmt.height;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    printf("default format, pixelformat:%s, width:%d, height:%d\n",
        "Motion-JPEG", this->default_fmt.width, this->default_fmt.height);

    setCaptureFormat(fd, &fmt);
    /*
     NOTE(if needed):
        可通过 struct v4l2_streamparm 结构体设置帧率
        struct v4l2_streamparm stream_param;
        stream_param.parm.capture.timeperframe.numerator;   // 分子(1)
        stream_param.parm.capture.timeperframe.denominator  // 分母(30)
    */
    struct v4l2_streamparm stream_param;
    stream_param.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_param.parm.capture.timeperframe.numerator = 1;
    stream_param.parm.capture.timeperframe.denominator = 30;
    if (-1 == ioctl(fd, VIDIOC_S_PARM, &stream_param)) {
        perror("set fps: ioctl VIDIOC_S_PARM error");
        return -1;
    }

    return	0;
}


int V4l2Video::initBuffer(int fd)
{
    struct v4l2_requestbuffers reqbuf;
    memset(&reqbuf, 0, sizeof(reqbuf));

    reqbuf.count = this->uCount;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf))
    {
        perror("ioctl VIDIOC_REQBUFS error");
        return	-1;
    }

    if ((reqbuf.count < 2) || (reqbuf.count > 5))
    {
        return	-1;
    }

    this->pBuffers = (VideoBuffer *)calloc(reqbuf.count, sizeof(VideoBuffer));
    if (NULL == this->pBuffers)
    {
        perror("calloc error");
        return	-1;
    }

    unsigned int numBufs = 0;
    for (numBufs = 0; numBufs < reqbuf.count; numBufs++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = numBufs;

        if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            perror("ioctl VIDIOC_QUERYBUF error");
            return	-1;
        }

        pBuffers[numBufs].uLength = buf.length;
        pBuffers[numBufs].pStart = (unsigned char *)mmap(NULL, buf.length, PROT_READ |
        PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (NULL == pBuffers[numBufs].pStart)
        {
            perror("buffer error");
            return  -1;
        }
    }
    qDebug() << "Init buffer successfully." << "\n";

    return	0;
}

int V4l2Video::startCapturing(int fd)
{
    unsigned int i = 0;
    for (i = 0; i < this->uCount; i ++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            perror("ioctl VIDIOC_QBUF error");
            return	-1;
        }
    }

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        perror("ioctl VIDIOC_STREAMON error");
        return	-1;
    }
    qDebug() << "Start capturing!!!!!!!!!!!!!" << "\n";

    return	0;
}

int V4l2Video::stopCapturing(int fd)
{
    enum v4l2_buf_type	type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        perror("ioctl VIDIOC_STREAMOFF error");
        return	-1;
    }

    unsigned int numBufs = 0;
    for (numBufs = 0; numBufs < this->uCount; numBufs++)
    {
        munmap(pBuffers[numBufs].pStart, pBuffers[numBufs].uLength);
        pBuffers[numBufs].pStart = NULL;
    }
    struct v4l2_requestbuffers reqbuf;
    memset(&reqbuf, 0, sizeof(reqbuf));

    reqbuf.count = 0;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf))
    {
        perror("ioctl VIDIOC_REQBUFS error");
        return	-1;
    }
    if (this->pBuffers) free(this->pBuffers);

    return	0;
}

int V4l2Video::checkVideoFd(int fd)
{
    // use select to check whether this fd is readable
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    int retval = select(fd + 1, &rfds, NULL, NULL, NULL);
    if (retval <= 0) {
        printf("select fail %d, %d %s\n", retval, errno, strerror(errno));
        return -1;
    }

    if (fd > 0 && FD_ISSET(fd, &rfds))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int V4l2Video::dequeueFrame(int fd, void **out_frame, int *frame_len, int *index)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("ioctl VIDIOC_DQBUF error");
        return	-1;
    }

    *out_frame = this->pBuffers[buf.index].pStart;
    *frame_len = this->pBuffers[buf.index].uLength;
    //this->uIndex = buf.index;
    *index = buf.index;

    struct timespec ts;
    int fps;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t last_frame_pts = this->pts;
    this->pts = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
    if (last_frame_pts != 0) {
        fps = 1000000ULL / (this->pts - last_frame_pts);
        qDebug() << "pts: " << this->pts
            << "true fps: " << fps << "\n";
    }

    return	0;
}

int V4l2Video::queueFrame(int fd, int index)
{
    if (-1 != index)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;

        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            return	-1;
        }

        return	0;
    }

    return	-1;
}
