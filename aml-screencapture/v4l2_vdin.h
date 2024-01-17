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
#define _AML_V4L2_VDIN_H_
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/videodev2.h>
#include <sys/time.h>
#include <mutex>
#include <map>
#include <thread>

#define VIDEO_CAPTURE_WIDTH     1920
#define VIDEO_CAPTURE_HEIGHT    1080
#define NB_BUFFER 6

struct VideoInfo {
    struct v4l2_capability cap;
    struct v4l2_format format;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers rb;
    long *mem[NB_BUFFER];
    unsigned canvas[NB_BUFFER];
    unsigned refcount[NB_BUFFER];
    bool isStreaming;
    int width;
    int height;
    int formatIn;
    int framesizeIn;
};
enum State{
    START,
    STOP,
};
typedef struct aml_screen_buffer_info {
    long * buffer_mem;
    int    buffer_canvas;
    long   tv_sec;
    long   tv_usec;
} aml_screen_buffer_info_t;


typedef void (*app_data_callback)(void *user, aml_screen_buffer_info_t *buff_info);

class vdin_screen_source {
    public:
        vdin_screen_source();
        ~vdin_screen_source();
        int init();
        int start();
        int stop();
        /* set the width, the height and the pixel format
         *
         *
         * width: the width, in pixels
         * height: the height, in pixels
         * color_format: the pixel format
         *
         * return 0 if success, errorcode otherwise
         */
        int set_format(int width = 640, int height = 480, int color_format = V4L2_PIX_FMT_NV21);
        /* filled  buffer in the driver's incoming queue
         *
         *
         * ptr: address of buffer,which is used
         *
         *
         * return 0 if success, errorcode otherwise
         */
        int release_buffer(long* ptr);
        /* set the callback function of user
         *
         *
         * callback: callback function
         * user : address of user
         *
         * return 0 if success, errorcode otherwise
         */
        int set_data_callback(app_data_callback callback, void* user);
        /* set the frame rate of data from driver
         *
         *
         * frameRate: frame rate of data from driver
         *
         *
         * return 0 if success, errorcode otherwise
         */
        int set_frame_rate(int frameRate);
         /* set the port type of data from driver
         *
         *
         * portType: there are values
         * PORTTYPE_VALUE_VIDEO_ONLY_main_screen 0x11000000 //main screen
         * PORTTYPE_VALUE_VIDEO_OSD_main_screen 0x11000001 //main screen
         * PORTTYPE_VALUE_VIDEO_ONLY_second_screen 0x11000002 //second screen
         * PORTTYPE_VALUE_VIDEO_OSD_second_screen 0x11000003 //second screen
         *
         * return 0 if success, errorcode otherwise
         */
        int set_port_type(unsigned int portType);
    private:
        void workThread();
        int acquire_buffer(aml_screen_buffer_info_t *buff_info);
        int start_v4l2_device();
        int stop_v4l2_device();
        std::thread thread_;
        std::map<long *, int> bufs_;
        std::map<long *, int> temp_bufs_;
        int buffer_count_;
        int frame_width_;
        int frame_height_;
        int buffer_size_;
        volatile int state_;
        int pixel_format_;
        std::mutex mutex_;
        int handle_;
        struct VideoInfo *video_info_;
        app_data_callback callback_;
        void *user_;
        long * temp_[4];
};



