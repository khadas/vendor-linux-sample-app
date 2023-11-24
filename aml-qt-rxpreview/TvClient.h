/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef TVCLIENT_H
#define TVCLIENT_H

#include <map>
#include <memory>

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <TvCommon.h>


using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

class CTvEvent {
public:
    static const int TV_EVENT_COMMOM = 0;
    static const int TV_EVENT_SCANNER = 1;
    static const int TV_EVENT_EPG = 2;//EPG
    static const int TV_EVENT_SOURCE_SWITCH = 3;
    static const int TV_EVENT_SINGLE_DETECT = 4;
    static const int TV_EVENT_ADC_CALIBRATION = 5;
    static const int TV_EVENT_VGA = 6;//VGA
    static const int TV_EVENT_3D_STATE = 7;//3D
    static const int TV_EVENT_AV_PLAYBACK = 8;//PLAYBACK EVENT MSG
    static const int TV_EVENT_SERIAL_COMMUNICATION = 9;
    static const int TV_EVENT_SOURCE_CONNECT = 10;
    static const int TV_EVENT_HDMIRX_CEC = 11;
    static const int TV_EVENT_BLOCK = 12;
    static const int TV_EVENT_CC = 13; //close caption
    static const int TV_EVENT_VCHIP = 14; //VCHIP
    static const int TV_EVENT_HDMI_IN_CAP = 15;
    static const int TV_EVENT_UPGRADE_FBC = 16;
    static const int TV_EVENT_2d4G_HEADSET = 17;
    static const int TV_EVENT_AV = 18;
    static const int TV_EVENT_SUBTITLE = 19;
    static const int TV_EVENT_SCANNING_FRAME_STABLE = 20;
    static const int TV_EVENT_FRONTEND = 21;
    static const int TV_EVENT_RECORDER = 22;
    static const int TV_EVENT_RRT = 23;//RRT
    static const int TV_EVENT_EAS = 24;//EAS
    static const int TV_EVENT_AUDIO_CB = 25;
    static const int TV_EVENT_SIG_DV_ALLM = 26;

    CTvEvent(int type) {
        mEventType = type;
    }
    ~CTvEvent() {

    }
    int getEventType() {
        return mEventType;
    }
private:
    int mEventType;
};

namespace  TvEvent {
    //events
    class SignalDetectEvent: public CTvEvent {
    public:
        SignalDetectEvent() : CTvEvent(CTvEvent::TV_EVENT_SINGLE_DETECT)
        {
            mSourceInput = 0;
            mFmt = 0;
            mTrans_fmt = 0;
            mStatus = 0;
            mDviFlag = 0;
            mhdr_info = 0;
        }
        ~SignalDetectEvent() {}
        int mSourceInput;
        int mFmt;
        int mTrans_fmt;
        int mStatus;
        int mDviFlag;
        unsigned int mhdr_info;
    };

    class SignalDvAllmEvent: public CTvEvent {
    public:
        SignalDvAllmEvent() : CTvEvent(CTvEvent::TV_EVENT_SIG_DV_ALLM)
        {
            allm_mode = 0;
            it_content = 0;
            cn_type = 0;
        }
        ~SignalDvAllmEvent() {}
        int allm_mode;
        int it_content;
        int cn_type;
    };

    class SourceConnectEvent: public CTvEvent {
    public:
        SourceConnectEvent() : CTvEvent(CTvEvent::TV_EVENT_SOURCE_CONNECT)
        {
            mSourceInput = 0;
            connectionState = 0;
        }
        ~SourceConnectEvent() {}
        int mSourceInput;
        int connectionState;
    };

    class SourceSwitchEvent: public CTvEvent {
    public:
        SourceSwitchEvent() : CTvEvent(CTvEvent::TV_EVENT_SOURCE_SWITCH)
        {
            DestSourceInput = 0;
            DestSourcePortNum = 0;
        }
        ~SourceSwitchEvent() {}
        int DestSourceInput;
        int DestSourcePortNum;
    };
};

class TvClient : public BBinder, public IBinder::DeathRecipient {
public:
    class TvClientIObserver {
    public:
        TvClientIObserver() {};
        virtual ~TvClientIObserver() {};
        virtual void onTvClientEvent(CTvEvent &event) = 0;
    };

    class DeathNotifier: public IBinder::DeathRecipient {
    public:
        DeathNotifier() {}
        virtual void binderDied(const wp<IBinder> &who);
    };
    static sp<DeathNotifier> mDeathNotifier;

    enum {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_TV_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
        CMD_SET_TV_CB = IBinder::FIRST_CALL_TRANSACTION + 2,
        CMD_CLR_TV_CB = IBinder::FIRST_CALL_TRANSACTION + 3,
        EVT_SRC_CT_CB = IBinder::FIRST_CALL_TRANSACTION + 4,
        EVT_SIG_DT_CB = IBinder::FIRST_CALL_TRANSACTION + 5,
        DATA_SET_ACTION = IBinder::FIRST_CALL_TRANSACTION + 6,
        DATA_GET_ACTION = IBinder::FIRST_CALL_TRANSACTION + 7,
        CMD_DATA_REQ  = IBinder::FIRST_CALL_TRANSACTION + 8,
        EVT_SIG_DV_ALLM = IBinder::FIRST_CALL_TRANSACTION + 9,
    };

    TvClient();
    ~TvClient();
    void Release();
    static TvClient *GetInstance();
    int setTvClientObserver(TvClientIObserver *observer);
    int StartTv(tv_source_input_t source);
    int StopTv(tv_source_input_t source);
    int SetEdidVersion(tv_source_input_t source, int edidVer);
    int GetEdidVersion(tv_source_input_t source);
    int SetEdidData(tv_source_input_t source, char *dataBuf);
    int GetEdidData(tv_source_input_t source,char *dataBuf);
    int SetVdinWorkMode(vdin_work_mode_t vdinWorkMode);
    int GetCurrentSourceFrameHeight();
    int GetCurrentSourceFrameWidth();
    int GetCurrentSourceFrameFps();
    int GetCurrentSourceColorDepth();
    tvin_aspect_ratio_t GetCurrentSourceAspectRatio();
    tvin_color_fmt_t GetCurrentSourceColorFormat();
    tvin_color_range_t GetCurrentSourceColorRange();
    tvin_line_scan_mode_t GetCurrentSourceLineScanMode();
    int GetSourceConnectStatus(tv_source_input_t source);
    int GetSPDInfo(tv_source_input_t source, char* dataBuf, size_t datalen);
    int SetEdidBoostOn(int bBoostOn);
private:
    int SendMethodCall(char *CmdString);
    int SendDataRequest(char *CmdString, char *data, size_t datalen);
    static int HandSourceConnectEvent(const void *param);
    static int HandSignalDetectEvent(const void *param);
    static int HandSignalDvAllmEvent(const void *param);
    int SendTvClientEvent(CTvEvent &event);
    virtual void binderDied(const wp<IBinder> &who);

    static sp<TvClient> mInstance;
    std::map<int, TvClientIObserver *> mTvClientObserver;

    sp<IBinder> tvServicebinder;
    int tvServicebinderId;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
};


#ifdef __cplusplus
}
#endif
#endif
