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
#define LOG_TAG "ScreenCapture"
#include "Log.h"
#include "ScreenCapture.h"

static int saveBufferAsFile(void *buffer, size_t size, char *file)
{
    int fd = open(file, O_CREAT|O_RDWR, 0666);
    int wrSize = 0;
    int ret = 0;

    if (fd < 0) {
        return fd;
    }

    do {
        int ret = write (fd, (char *)buffer + wrSize, size - wrSize);
        if (ret > 0) {
            wrSize += ret;
        } else {
            break;
        }
    } while (wrSize < size);
    close(fd);
    return ret;
}

static void checkAndSaveBufferToFile(const char *baseFile, char *filename, void *buffer, size_t size)
{
    if (0 == access(baseFile, F_OK)) {
        if (saveBufferAsFile(buffer, size, filename) >= 0) {
            ALOGD("Save file ok: %s", filename);
        } else {
            ALOGW("Save file fail: %s!", filename);
        }
    }
}

static int getBufferSize(int width, int height, aml_capture_data_type data_type) {
    int buf_size = 0;
    switch (data_type) {
        case SCREENCONTROL_FMT_NV21:
        case SCREENCONTROL_FMT_NV12:
            {
                buf_size = width * height * 3 / 2;
                break;
            }
        case SCREENCONTROL_FMT_RGB565:
            buf_size = width * height * 2;
            break;
        case SCREENCONTROL_FMT_RGB24:
            buf_size = width * height * 3;
            break;
        case SCREENCONTROL_FMT_ARGB888:
        case SCREENCONTROL_FMT_ABGR888:
            buf_size = width * height * 4;
            break;
        default:
            ALOGD("[%s %d] Invalid format",__FUNCTION__, __LINE__);
            buf_size = width * height * 3 / 2;
    }
    return buf_size;

}
static void VdinDataCallBack(void *user, aml_screen_buffer_info_t *buffer){
    ScreenCapture *source = static_cast<ScreenCapture *>(user);
    source->dataCallBack(buffer);
    return;
}
ScreenCapture::ScreenCapture()
    :source_(std::make_unique<vdin_screen_source>()),
    width_(0),
    height_(0),
    buffer_size_(0),
    frame_rate_(0),
    data_type_(SCREENCONTROL_UNKNOWN_TYPE),
    port_type_(PORTTYPE_VALUE_VIDEO_OSD_main_screen),
    started_(false) {

    }
ScreenCapture::~ScreenCapture() {

}
bool  ScreenCapture::init(ScreenCapture_Params *params)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ALOGD("[%s %d] width=%d ,height=%d,framerate=%d,data_type=%d,params->port_type=%d",
    __FUNCTION__, __LINE__,params->width,params->height,params->framerate,params->data_type,params->port_type);
    width_ = params->width;
    height_ = params->height;
    frame_rate_ = params->framerate;
    data_type_ = params->data_type;
    port_type_ = params->port_type;
    buffer_size_ = getBufferSize(width_,height_,data_type_);
    return true;
}

bool ScreenCapture::start()
{
    if (source_->init() != 0)
        return false;
    if (source_->set_port_type(port_type_) != 0)
        return false;
    if (source_->set_frame_rate(frame_rate_) != 0)
        return false;
    int format = 0;
    if (data_type_ == SCREENCONTROL_FMT_NV21) {
        format = V4L2_PIX_FMT_NV21;
    }else if (data_type_ == SCREENCONTROL_FMT_NV12) {
        format = V4L2_PIX_FMT_NV12;
    }else if (data_type_ == SCREENCONTROL_FMT_ARGB888) {
        format = V4L2_PIX_FMT_RGB32;
    }else if (data_type_ == SCREENCONTROL_FMT_ABGR888) {
        format = V4L2_PIX_FMT_BGR32;
    }else if (data_type_ == SCREENCONTROL_FMT_RGB565) {
        format = V4L2_PIX_FMT_RGB565;
    }else if (data_type_ == SCREENCONTROL_FMT_RGB24 ) {
        format = V4L2_PIX_FMT_RGB24;
    }else {
        ALOGD("[%s %d] Invalid data_type",__FUNCTION__, __LINE__);
        return false;
    }
    source_->set_format(width_,height_,format);
    if (source_->set_data_callback(VdinDataCallBack,(void*) this) != 0)
        return false;
    if (source_->start() != 0)
        return false;
    started_ = true;
    ALOGD("[%s %d] finish!",__FUNCTION__, __LINE__);
    return true;
}

bool ScreenCapture::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ALOGD("[%s %d] begin",__FUNCTION__, __LINE__);
    started_ = false;
    source_->stop();
    ALOGD("[%s %d] over",__FUNCTION__, __LINE__);
    return true;
}

bool ScreenCapture::readBuffer(void *data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (started_ && !buffer_list_.empty()) {
        std::unique_ptr<uint8_t[]> buffer = std::move(buffer_list_.front());
        buffer_list_.pop_front();
        if (buffer != nullptr && data != nullptr) {
            memcpy(data, buffer.get(), buffer_size_);
            return true;
        }else {
            ALOGD("[%s %d] buffer or data invalid",__FUNCTION__, __LINE__);
        }
    }
    return false;
}

void ScreenCapture::dataCallBack(aml_screen_buffer_info_t *data) {
    std::lock_guard<std::mutex> lock(mutex_);
    ALOGD("[%s %d] started_:%d",__FUNCTION__, __LINE__, started_);
    if (started_) {
        auto buffer = std::make_unique<uint8_t[]>(buffer_size_);
        memcpy(buffer.get(), data->buffer_mem, buffer_size_);
        buffer_list_.push_back(std::move(buffer));
        source_->release_buffer(data->buffer_mem);
    }

}


