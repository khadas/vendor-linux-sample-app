#ifndef PAGE3_H
#define PAGE3_H

#include <QtWidgets>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtWidgets>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

class Page3 : public QWidget {
    Q_OBJECT

public:
  Page3(QWidget *parent = nullptr);
  ~Page3();


private:
    QLabel          *pLabelPage1;
    QLabel          *plabelPhoto1_1;
    QSlider         *psliderPage1;

public slots:
    bool onSliderValueChanged(int brightness);

};


#endif


