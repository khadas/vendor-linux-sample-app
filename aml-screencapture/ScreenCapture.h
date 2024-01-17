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

#define V_AML_ScreenCapture_H_

#include "v4l2_vdin.h"
#include <list>

#define PORTTYPE_VALUE_VIDEO_ONLY_main_screen 0x11000000 //main screen
#define PORTTYPE_VALUE_VIDEO_OSD_main_screen 0x11000001 //main screen
#define PORTTYPE_VALUE_VIDEO_ONLY_second_screen 0x11000002 //second screen
#define PORTTYPE_VALUE_VIDEO_OSD_second_screen 0x11000003 //second screen

enum aml_capture_data_type{
    SCREENCONTROL_UNKNOWN_TYPE,
    SCREENCONTROL_FMT_NV21,
    SCREENCONTROL_FMT_NV12,
    SCREENCONTROL_FMT_ARGB888,
    SCREENCONTROL_FMT_ABGR888,
    SCREENCONTROL_FMT_RGB565,
    SCREENCONTROL_FMT_RGB24,
};


typedef struct _ScreenCapture_Params
{
    int width;
    int height;
    int framerate;
    enum  aml_capture_data_type data_type;
    int port_type;
}ScreenCapture_Params;


class ScreenCapture {
public:
    ScreenCapture();
    virtual ~ScreenCapture();
    bool init(ScreenCapture_Params *params);
    bool start();
    bool stop();
    /* get rgb data from output's list(buffer_list_)
    *
    *
    * data: the address of data ,need to copy
    *
    * return true if success, errorcode otherwise
    */
    bool readBuffer(void *data);
    /* it's be called if callback function which is be set to vdin_screen_source
    *
    *
    * buffer: the buffer info of output rgb data
    *
    * return true if success, errorcode otherwise
    */
    void dataCallBack(aml_screen_buffer_info_t *buffer);

private:
    std::unique_ptr<vdin_screen_source> source_;
    std::mutex mutex_;
    int width_;
    int height_;
    int buffer_size_;
    int frame_rate_;
    int port_type_;
    aml_capture_data_type data_type_;
    std::list<std::unique_ptr<uint8_t[]>> buffer_list_;
    bool started_;

};



