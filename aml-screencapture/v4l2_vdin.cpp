/*
 * Copyright (C) 2022 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "v4l2_vdin"
#include "Log.h"
#include "v4l2_vdin.h"


#define V4L2_ROTATE_ID 0x980922

#define boundary 32
#define ALIGN_32(x) ((x + (boundary) - 1)& ~((boundary) - 1))
#define ALIGN(b,w) (((b)+((w)-1))/(w)*(w))
static int getBufSize(int format, int width, int height)
{
    int buf_size = 0;

    switch (format) {
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        {
            buf_size = width * height * 3 / 2;
            break;
        }
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_RGB565:
        case V4L2_PIX_FMT_RGB565X:
            buf_size = width * height * 2;
            break;
        case V4L2_PIX_FMT_RGB24:
            buf_size = width * height * 3;
            break;
        case V4L2_PIX_FMT_RGB32:
        case V4L2_PIX_FMT_BGR32:
            buf_size = width * height * 4;
            break;
        default:
            ALOGD("[%s %d] Invalid format",__FUNCTION__, __LINE__);
            buf_size = width * height * 3 / 2;
    }
    return buf_size;
}
static void two_bytes_per_pixel_memcpy_align32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int stride = (width + 31) & ( ~31);
        int h;
        for (h=0; h<height; h++)
        {
                memcpy( dst, src, width*2);
                dst += width*2;
                src += stride*2;
        }
}
static void nv21_memcpy_align32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int stride = (width + 31) & ( ~31);
        int h;
        for (h=0; h<height*3/2; h++)
        {
                memcpy( dst, src, width);
                dst += width;
                src += stride;
        }
}

static void yv12_memcpy_align32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int new_width = (width + 63) & ( ~63);
        int stride;
        int h;
        for (h=0; h<height; h++)
        {
                memcpy( dst, src, width);
                dst += width;
                src += new_width;
        }

        stride = ALIGN(width/2, 16);
        for (h=0; h<height; h++)
        {
                memcpy( dst, src, width/2);
                dst += stride;
                src += new_width/2;
        }
}

static void rgb24_memcpy_align32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int stride = (width + 31) & ( ~31);
        int  h;
        for (h=0; h<height; h++)
        {
                memcpy( dst, src, width*3);
                dst += width*3;
                src += stride*3;
        }
}

static void rgb32_memcpy_align32(unsigned char *dst, unsigned char *src, int width, int height)
{
        int stride = (width + 31) & ( ~31);
        int h;
        for (h=0; h<height; h++)
        {
                memcpy( dst, src, width*4);
                dst += width*4;
                src += stride*4;
        }
}
vdin_screen_source::vdin_screen_source()
                  : buffer_count_(1),
                    frame_width_(VIDEO_CAPTURE_WIDTH),
                    frame_height_(VIDEO_CAPTURE_HEIGHT),
                    buffer_size_(0),
                    state_(STOP),
                    pixel_format_(V4L2_PIX_FMT_NV21),
                    handle_(-1),
                    video_info_(NULL),
                    callback_(0),
                    user_(NULL)

{
    temp_[0] = NULL;
    temp_[1] = NULL;
    temp_[2] = NULL;
    temp_[3] = NULL;
}

vdin_screen_source::~vdin_screen_source()
{
    if (video_info_)
        free (video_info_);
    if (handle_ >= 0) {
        close(handle_);
        handle_ = -1;
    }

}

int vdin_screen_source::start_v4l2_device()
{
    int ret = -1;

    video_info_ = (struct VideoInfo *) calloc (1, sizeof (struct VideoInfo));
    if (video_info_ == NULL)
    {
        ALOGD("[%s %d] no memory for video_info_",__FUNCTION__, __LINE__);
        close(handle_);
        handle_ = -1;
        return ret;
    }

    ret = ioctl(handle_, VIDIOC_QUERYCAP, &video_info_->cap);
    if (ret < 0) {
        ALOGD("[%s %d] VIDIOC_QUERYCAP:%d handle_:%x",__FUNCTION__, __LINE__,ret,handle_);
        return ret;
    }

    video_info_->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_info_->rb.memory = V4L2_MEMORY_MMAP;
    video_info_->rb.count = buffer_count_;

    ret = ioctl(handle_, VIDIOC_REQBUFS, &video_info_->rb);

    if (ret < 0) {
        ALOGD("[%s %d] VIDIOC_REQBUFS:%d handle_:%x",__FUNCTION__, __LINE__,ret,handle_);
        return ret;
    }

    for (int i = 0; i < buffer_count_; i++) {
        memset (&video_info_->buf, 0, sizeof (struct v4l2_buffer));

        video_info_->buf.index = i;
        video_info_->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        video_info_->buf.memory = V4L2_MEMORY_MMAP;

        ret = ioctl (handle_, VIDIOC_QUERYBUF, &video_info_->buf);
        if (ret < 0) {
            ALOGD("[%s %d] VIDIOC_QUERYBUF %d failed",__FUNCTION__, __LINE__,i);
            return ret;
        }

        video_info_->canvas[i] = video_info_->buf.reserved;
        video_info_->mem[i] = (long *)mmap (0, video_info_->buf.length, PROT_READ | PROT_WRITE,
               MAP_SHARED, handle_, video_info_->buf.m.offset);

        if (video_info_->mem[i] == MAP_FAILED) {
            ALOGD("[%s %d] MAP_FAILED %d",__FUNCTION__, __LINE__,i);
            return -1;
        }
        video_info_->refcount[i] = 0;

        auto r = bufs_.insert({video_info_->mem[i],i});
        if (!r.second) {
            ALOGD("[%s %d] bufs_ insert fail %d",__FUNCTION__, __LINE__,i);
            return -1;

        }
        if (frame_width_ %32  != 0) {
            auto r =temp_bufs_.insert({temp_[i],i});
            if (!r.second) {
                ALOGD("[%s %d] temp_bufs_ insert fail %d",__FUNCTION__, __LINE__,i);
                return -1;
            }

        }
    }

    for (int i = 0; i < buffer_count_; i++) {
        video_info_->buf.index = i;
        video_info_->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        video_info_->buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(handle_, VIDIOC_QBUF, &video_info_->buf);
        if (ret < 0) {
            ALOGD("[%s %d] VIDIOC_QBUF Failed",__FUNCTION__, __LINE__);
            return -1;
        }
    }

    enum v4l2_buf_type bufType;
    bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl (handle_, VIDIOC_STREAMON, &bufType);

    return ret;
}

int vdin_screen_source::stop_v4l2_device()
{
    int ret;
    enum v4l2_buf_type bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl (handle_, VIDIOC_STREAMOFF, &bufType);
    if (ret < 0) {
        ALOGD("[%s %d] StopStreaming: Unable to stop capture: %s",__FUNCTION__, __LINE__,strerror(errno));
    }
    for (int i = 0; i < buffer_count_; i++) {
        if (munmap(video_info_->mem[i], video_info_->buf.length) < 0) {
            ALOGD("[%s %d] Unmap failed:%d: %s",__FUNCTION__, __LINE__,i,strerror(errno));
        }
    }
    bufs_.clear();
    if (frame_width_ %32  != 0) {
        temp_bufs_.clear();
        for (int j = 0; j <buffer_count_; j++ ) {
            free(temp_[j]);
            temp_[j] = NULL;
        }
    }
    return ret;
}
int vdin_screen_source::init() {
    int ret = -1;
    handle_ = open("/dev/video12", O_RDWR| O_NONBLOCK);
    if (handle_ < 0)
    {
        ALOGD("[%s %d] /dev/video12 handle_:%x [%s]",__FUNCTION__, __LINE__,handle_,strerror(errno));
        return ret;
    }
    video_info_ = (struct VideoInfo *) calloc (1, sizeof (struct VideoInfo));
    if (video_info_ == NULL)
    {
        ALOGD("[%s %d] no memory for video_info_",__FUNCTION__, __LINE__);
        close(handle_);
        handle_ = -1;
        return ret;
    }
    buffer_count_ = 4;
    pixel_format_ = V4L2_PIX_FMT_NV21;
    frame_width_ = VIDEO_CAPTURE_WIDTH;
    frame_height_ = VIDEO_CAPTURE_HEIGHT;
    buffer_size_ = frame_width_ * frame_height_ * 3/2;
    state_ = STOP;
    callback_ = NULL;
    ALOGD("[%s %d] success.",__FUNCTION__, __LINE__);
    return 0;
}
int vdin_screen_source::start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    int ret;

    if (state_ == START) {
        ALOGD("[%s %d] already state state_=%d",__FUNCTION__, __LINE__,state_);
        return -1;
    }

    ret = start_v4l2_device();
    if (ret != 0) {
        ALOGD("[%s %d] Start v4l2 device failed:%d",__FUNCTION__, __LINE__,ret);
        return ret;
    }
    thread_ = std::thread([this]{ workThread(); });
    state_ = START;

    ALOGD("[%s %d] success.",__FUNCTION__, __LINE__);
    return 0;
}

int vdin_screen_source::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ALOGD("[%s %d] begin",__FUNCTION__, __LINE__);
    int ret;
    state_ = STOP;
    thread_.join();
    enum v4l2_buf_type bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl (handle_, VIDIOC_STREAMOFF, &bufType);
    if (ret < 0) {
        ALOGD("[%s %d] StopStreaming: Unable to stop capture: %s",__FUNCTION__, __LINE__,strerror(errno));
    }
    for (int i = 0; i < buffer_count_; i++) {
        if (munmap(video_info_->mem[i], video_info_->buf.length) < 0)
            ALOGD("[%s %d] StopStreaming: munmap fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    bufs_.clear();
    if (frame_width_ %32  != 0) {
        temp_bufs_.clear();
        for (int j = 0; j <buffer_count_; j++ ) {
            free(temp_[j]);
            temp_[j] = NULL;
        }
    }

    buffer_count_ = 0;
    ALOGD("[%s %d] finish",__FUNCTION__, __LINE__);
    return ret;
}

int vdin_screen_source::set_data_callback(app_data_callback callback, void* user)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ALOGD("[%s %d] begin",__FUNCTION__, __LINE__);
    if (callback == NULL) {
        ALOGD("[%s %d] NULL data callback pointer",__FUNCTION__, __LINE__);
        return -1;
    }
    callback_ = callback;
    user_ = user;
    return 0;
}

int vdin_screen_source::set_format(int width, int height, int color_format)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ == START)
        return 0;
    int ret;
    video_info_->width = ALIGN_32(width);
    video_info_->height = height;
    video_info_->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_info_->format.fmt.pix.width = ALIGN_32(width);
    video_info_->format.fmt.pix.height = height;
    video_info_->format.fmt.pix.pixelformat = color_format;
    pixel_format_ = color_format;
    if (pixel_format_ == V4L2_PIX_FMT_RGB32 || pixel_format_ == V4L2_PIX_FMT_BGR32) {
          buffer_count_ = 3;
    }
    for (int i = 0; i < buffer_count_; i++) {
        temp_[i] = NULL;
    }
    frame_width_ = width;
    frame_height_ = height;
    buffer_size_ = getBufSize(color_format, ALIGN_32(width), frame_height_);

    if (frame_width_ %32  != 0) {
        ALOGD("[%s %d] frame_width_ %d (frame_width_ 32)",__FUNCTION__, __LINE__,frame_width_);
        for (int i = 0; i < buffer_count_; i++) {
            temp_[i] = (long*) malloc(buffer_size_);
            if (!temp_[i]) {
                ALOGD("[%s %d] malloc temp_ buffer failed",__FUNCTION__, __LINE__);
                return -1;
            }
        }
    }
    ret = ioctl(handle_, VIDIOC_S_FMT, &video_info_->format);
    if (ret < 0) {
        ALOGD("[%s %d] VIDIOC_S_FMT %d",__FUNCTION__, __LINE__,ret);
        return ret;
    }
    return ret;
}


int vdin_screen_source::set_frame_rate(int frameRate)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int ret = 0;

    if (handle_<0)
        return -1;

    struct v4l2_streamparm sparm;
    memset(&sparm, 0, sizeof( sparm ));
    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//stream_flag;
    sparm.parm.output.timeperframe.denominator = frameRate;
    sparm.parm.output.timeperframe.numerator = 1;
    ret = ioctl(handle_, VIDIOC_S_PARM, &sparm);
    if (ret < 0) {
        ALOGD("[%s %d] Set frame rate fail: %s. ret=%d",__FUNCTION__, __LINE__,strerror(errno),ret);
    }
    return ret ;
}


int vdin_screen_source::set_port_type(unsigned int portType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int ret = 0;
    ret = ioctl(handle_, VIDIOC_S_INPUT, &portType);
    if (ret < 0) {
        ALOGD("[%s %d] Set port type fail: %s. ret=%d",__FUNCTION__, __LINE__,strerror(errno),ret);
    }
    return ret;
}


int vdin_screen_source::acquire_buffer(aml_screen_buffer_info_t *buff_info)
{
    int ret = -1;
    video_info_->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_info_->buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(handle_, VIDIOC_DQBUF, &video_info_->buf);
    if (ret < 0) {
        if (EAGAIN == errno) {
            ret = -EAGAIN;
        }
        buff_info->buffer_mem    = 0;
        buff_info->buffer_canvas = 0;
        ALOGD("[%s %d] VIDIOC_DQBUF failed!!!",__FUNCTION__, __LINE__);
        return ret;
    }
    ALOGD("[%s %d] VIDIOC_DQBUF Get V4l2 buffer!!!",__FUNCTION__, __LINE__);
    if (frame_width_ %32  != 0) {
        switch (video_info_->format.fmt.pix.pixelformat) {
            case V4L2_PIX_FMT_YVU420:
                yv12_memcpy_align32((unsigned char*)temp_[video_info_->buf.index], (unsigned char*)video_info_->mem[video_info_->buf.index], frame_width_, frame_height_);
                break;
            case V4L2_PIX_FMT_NV21:
	    case V4L2_PIX_FMT_NV12:
                nv21_memcpy_align32((unsigned char*)temp_[video_info_->buf.index], (unsigned char*)video_info_->mem[video_info_->buf.index], frame_width_, frame_height_);
                break;
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_RGB565:
            case V4L2_PIX_FMT_RGB565X:
                two_bytes_per_pixel_memcpy_align32 ((unsigned char*)temp_[video_info_->buf.index], (unsigned char*)video_info_->mem[video_info_->buf.index], frame_width_, frame_height_);
                break;
            case V4L2_PIX_FMT_RGB24:
                rgb24_memcpy_align32((unsigned char*)temp_[video_info_->buf.index], (unsigned char*)video_info_->mem[video_info_->buf.index], frame_width_, frame_height_);
                break;
            case V4L2_PIX_FMT_RGB32:
            case V4L2_PIX_FMT_BGR32:
                rgb32_memcpy_align32((unsigned char*)temp_[video_info_->buf.index], (unsigned char*)video_info_->mem[video_info_->buf.index], frame_width_, frame_height_);
                break;
            default:
                ALOGD("[%s %d] Invalid format",__FUNCTION__, __LINE__);
                break;
        }
        buff_info->buffer_mem    = temp_[video_info_->buf.index];
        buff_info->buffer_canvas = video_info_->canvas[video_info_->buf.index];
        buff_info->tv_sec        = video_info_->buf.timestamp.tv_sec;
        buff_info->tv_usec       = video_info_->buf.timestamp.tv_usec;
    } else {
        buff_info->buffer_mem    = video_info_->mem[video_info_->buf.index];
        buff_info->buffer_canvas = video_info_->canvas[video_info_->buf.index];
        buff_info->tv_sec        = video_info_->buf.timestamp.tv_sec;
        buff_info->tv_usec       = video_info_->buf.timestamp.tv_usec;
    }
    ALOGD("[%s %d] get the data successful",__FUNCTION__, __LINE__);
    return ret;
}

int vdin_screen_source::release_buffer(long* ptr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int ret = -1;
    int currentIndex;
    v4l2_buffer hbuf_query;
    if (frame_width_ % 32 != 0) {
        currentIndex = temp_bufs_[ptr];
    } else {
        currentIndex = bufs_[ptr];
    }
    if (video_info_->refcount[currentIndex] > 0) {
        video_info_->refcount[currentIndex] -= 1;
    } else {
        ALOGD("[%s %d] return buffer when refcount already zero",__FUNCTION__, __LINE__);
        return 0;
    }
    if (video_info_->refcount[currentIndex] == 0) {
        memset(&hbuf_query,0,sizeof(v4l2_buffer));
        hbuf_query.index = currentIndex;
        hbuf_query.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        hbuf_query.memory = V4L2_MEMORY_MMAP;
        ALOGD("[%s %d] ioctl VIDIOC_QBUF",__FUNCTION__, __LINE__);
        ret = ioctl(handle_, VIDIOC_QBUF, &hbuf_query);
        if (ret != 0) {
            ALOGD("[%s %d] Return Buffer :%d failed",__FUNCTION__, __LINE__,currentIndex);
        }
    }
    return 0;
}

void vdin_screen_source::workThread()
{
    int index;
    aml_screen_buffer_info_t buff_info;
    int ret;
    long *src = NULL;

    ALOGD("[%s %d] start.",__FUNCTION__, __LINE__);

	while (state_ == START) {
        usleep(5 * 1000);
        ALOGD("[%s %d] acquire_buffer before.",__FUNCTION__, __LINE__);
        ret = acquire_buffer(&buff_info);
        ALOGD("[%s %d] acquire_buffer done.",__FUNCTION__, __LINE__);
        if (ret != 0 || (buff_info.buffer_mem == 0)) {
            ALOGD("[%s %d] Get V4l2 buffer failed, ret:%d",__FUNCTION__, __LINE__, ret);
            continue;
        }
        ALOGD("[%s %d] Get V4l2 buffer!!!",__FUNCTION__, __LINE__);
        src = buff_info.buffer_mem;
        if (frame_width_ % 32 != 0) {
            index = temp_bufs_[src];
        } else {
            index = bufs_[src];
        }
        ALOGD("[%s %d] callback_:%d state_:%d",__FUNCTION__, __LINE__, callback_, state_);
        if (callback_ != NULL&& state_ == START) {
            video_info_->refcount[index] += 1;
            ALOGD("[%s %d] callback_:%d state_:%d",__FUNCTION__, __LINE__, callback_, state_);
            callback_(user_, &buff_info);
        }
    }
    return;
}


