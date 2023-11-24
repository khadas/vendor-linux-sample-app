#ifndef PAGE1_H
#define PAGE1_H

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
#include "QtRx.h"
#include "Whiteboard.h"

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

class Page1 : public QWidget {
    Q_OBJECT

public:
    Page1(QWidget *parent = nullptr);
    QWidget *parent;

protected:

    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "enter mousePressEvent";
        const std::string filePath = "/sys/kernel/debug/dri/0/vpu/blank";
        std::ifstream inputFile(filePath);
        int value = 2;

        if (!inputFile.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
        }

        std::string fileContent;

        // Read the file content line by line
        std::string line;
        while (std::getline(inputFile, line)) {
            fileContent += line + '\n';
        }

        inputFile.close();

        // Output the complete file content
        std::cout << "File content:\n" << fileContent << std::endl;

        // Find specific information in the file content
        if (fileContent.find("blank_enable: 1") != std::string::npos) {
            std::cout << "OSD plane is blanked." << std::endl;
            value=1;
        } else if (fileContent.find("blank_enable: 0") != std::string::npos) {
            std::cout << "OSD plane is unblanked." << std::endl;
        } else {
            std::cerr << "Unable to determine OSD plane status." << std::endl;
        }

        if ((event->button() == Qt::LeftButton) && (1 == value)) {

            WriteSysfs("/sys/kernel/debug/dri/0/vpu/blank", 0);
            qDebug() << "enter DisplayInit";
        }
    }

    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Clear); // Set transparent mode
        painter.fillRect(rectangleToClear, Qt::transparent); // Fill transparent area
    }

private:
    QLabel          *pLabelPage2;
    QPushButton     *pHdmi2_1;
    QPushButton     *pHdmi2_2;
    QPushButton     *pHdmi2_3;
    QPushButton     *pExit2_1;
    QPushButton     *pUndoButton;
    QPushButton     *pClearButton;
    QPushButton     *pFullScreen2_1;
    QRect rectangleToClear; // Declare a member variable
    QtRx *pRxtest;
    Whiteboard *pDrawingBoard;

public slots:
    void Selet_HDMI1();
    void Selet_HDMI2();
    void Selet_HDMI3();
    void Selet_HDMIQ();
    void FullScreen();
};

#endif
