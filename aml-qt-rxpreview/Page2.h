#ifndef PAGE2_H
#define PAGE2_H

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
#include "Whiteboard.h"


class Page2 : public QWidget {
    Q_OBJECT

public:
  Page2(QWidget *parent = nullptr);
  ~Page2();

private:
    QPushButton     *pUndoButton;
    QPushButton     *pClearButton;
    QLabel          *pLabelPage2;
    Whiteboard      *pdrawingBoard;

};

#endif

