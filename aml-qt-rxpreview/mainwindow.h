#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QtGui>
#include <QtWidgets/QLabel>
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QMutex>
#include <QStackedWidget>
#include <QPushButton>
#include <QSlider>
#include <fstream>
#include <iostream>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include "ImageDisplayThread.h"
#include "Whiteboard.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);


signals:

private slots:
    void onButtonPage1Clicked();
    void onButtonPage2Clicked();
    void onButtonPage3Clicked();
    void onButtonPage4Clicked();

private:
    QStackedWidget *stackedWidget;
    QWidget         *page1;
    QWidget         *page2;
    QWidget         *page3;
    QWidget         *page4;
    QLabel          *plabelPhoto1_1;
    QPushButton     *pHdmi2_1;
    QPushButton     *pHdmi2_2;
    QPushButton     *pHdmi2_3;
    QPushButton     *pExit2_1;
    QPushButton     *pFullScreen2_1;
    QLabel          *pLabelPage1;
    QLabel          *pLabelPage4;
    QLabel         * videoWidget2;

};

#endif
