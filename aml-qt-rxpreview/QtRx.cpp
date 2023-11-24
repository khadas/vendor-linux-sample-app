// ImageDisplayThread.cpp
#include "QtRx.h"


QtRx::QtRx()
{
	mpTvClient = TvClient::GetInstance();
	mpTvClient->setTvClientObserver(this);
}

QtRx::~QtRx()
{
    mpTvClient->Release();
}

static int WriteSysfs(const char *path, const char *cmd)
{
    int fd;
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0777);

    if (fd >= 0) {
        write(fd, cmd, strlen(cmd));
        close(fd);
        return 0;
    }

    return -1;
}

static int WriteSysfs(const char *path, int value)
{
    char cmdbuf[32] = {0};
    sprintf(cmdbuf,"%d",value);
    return WriteSysfs(path, cmdbuf);
}


void QtRx::onTvClientEvent(CTvEvent &event)
{
    int eventType = event.getEventType();
    printf("%s: eventType: %d.\n", __FUNCTION__, eventType);
    switch (eventType) {
    case CTvEvent::TV_EVENT_SINGLE_DETECT: {
        TvEvent::SignalDetectEvent *signalDetectEvent = (TvEvent::SignalDetectEvent *)(&event);
        printf("%s: source: %d, signalFmt: %d, transFmt: %d, status: %d, isDVI: %d, Hdrinfo: %ud.\n", __FUNCTION__,
                signalDetectEvent->mSourceInput,
                signalDetectEvent->mFmt,
                signalDetectEvent->mTrans_fmt,
                signalDetectEvent->mStatus,
                signalDetectEvent->mDviFlag,
                signalDetectEvent->mhdr_info);
        WriteSysfs("/sys/class/video/disable_video", 0);
        break;
    }
    case CTvEvent::TV_EVENT_SOURCE_CONNECT: {
        TvEvent::SourceConnectEvent *sourceConnectEvent = (TvEvent::SourceConnectEvent *)(&event);
		printf("%s: source: %d, connectStatus: %d\n", __FUNCTION__,
        sourceConnectEvent->mSourceInput, sourceConnectEvent->connectionState);
		break;
    }
    default:
        printf("invalid event!\n", __FUNCTION__);
        break;
    }

    return;
}

int QtRx::SendCmd(const char *data) {
    printf("%s: cmd is %s.\n", __FUNCTION__, data);
    if (strcmp(data, "start") == 0) {
        mpTvClient->StartTv(CurrentSource);
    } else if (strcmp(data, "stop") == 0) {
        mpTvClient->StopTv(CurrentSource);
    } else {
        LOGE("invalid cmd!\n");
    }

    return 0;
}


