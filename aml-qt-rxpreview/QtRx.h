#ifndef QTRX_H
#define QTRX_H

// ImageDisplayThread.h
//tong add
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtWidgets>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <CTvClientLog.h>
#include "TvClient.h"

class QtRx: public TvClient::TvClientIObserver {
	public:
		tv_source_input_t CurrentSource;
        int EdidVer;
		QtRx();

		~QtRx();

		void onTvClientEvent(CTvEvent &event);

		int SendCmd(const char *data);

		TvClient *mpTvClient;

};

#endif

