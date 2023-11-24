// ImageDisplayThread.cpp
#include <QtWidgets>
#include <QLabel>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include "Page1.h"
#include "Whiteboard.h"

Page1::Page1(QWidget *parent) {
    this->parent = parent;

    // Apply a mature and subdued stylesheet to buttons
    QString buttonStylesheet = "QPushButton {"
                               "    background-color: #34495e;"
                               "    border: none;"
                               "    color: white;"
                               "    padding: 10px 20px;"
                               "    text-align: center;"
                               "    text-decoration: none;"
                               "    display: inline-block;"
                               "    font-size: 30px;"
                               "    border-radius: 8px;"
                               "    box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1);"
                               "}"
                               "QPushButton:hover {"
                               "    background-color: #2c3e50;"
                               "    cursor: pointer;"
                               "}"
                               "QPushButton:pressed {"
                               "    background-color: #273746;"
                               "    box-shadow: inset 0px 2px 4px rgba(0, 0, 0, 0.1);"
                               "}";

    QString backgroundStylesheet = "background-color: #c0c0c0;";

    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();
    // Get the size of the primary screen (assuming it's the 4K screen)
    QSize screenSize = screen->size();

    setFixedSize(screenSize.width(), screenSize.height());

    rectangleToClear = QRect(0, 0, screenSize.width(), screenSize.height()); // For example, define a position and size
    this->pDrawingBoard = new Whiteboard(this);
    this->pDrawingBoard->resize(screenSize.width(), screenSize.height()); // Resize the board to fill the screen while maintaining aspect ratio
    this->pDrawingBoard->move(0, 0); // Set the position and size of the board
    // Set the page style sheet to achieve a transparent background
    this->pDrawingBoard->setStyleSheet("background-color: rgba(0, 0, 0, 0);"); // rgba(0, 0, 0, 0) represents a completely transparent background

    // "Page 1 Content" label
    this->pLabelPage2 = new QLabel("HDMIIN", this);
    pLabelPage2->move(0, 0); // Set the position of the label, x-coordinate is 100, y-coordinate is 50
    // Set the font size of the label
    this->pLabelPage2->setFont(QFont("Arial", screenSize.width() / 96)); // Use Arial font in the example, font size in pixels
    // Set the size of the label to fit the text size and add extra padding
    this->pLabelPage2->setFixedSize(this->pLabelPage2->fontMetrics().boundingRect("HDMIIN").size() + QSize(20, 10));

    // Create two buttons and set their absolute positions
    this->pHdmi2_1 = new QPushButton("HDMI1", this);
    this->pHdmi2_1->setStyleSheet(buttonStylesheet);
    this->pHdmi2_1->move(screenSize.width() / 16 * 14, screenSize.height() / 108 * 12); // Set the position of the first button, x-coordinate is 100, y-coordinate is 100
    this->pHdmi2_1->resize(screenSize.width() / 15, screenSize.height() / 108 * 4);

    this->pHdmi2_2 = new QPushButton("HDMI2", this);
    this->pHdmi2_2->setStyleSheet(buttonStylesheet);
    this->pHdmi2_2->move(screenSize.width() / 16 * 14, screenSize.height() / 108 * 17); // Set the position of the second button, x-coordinate is 300, y-coordinate is 200
    this->pHdmi2_2->resize(screenSize.width() / 15, screenSize.height() / 108 * 4);

    // Create two buttons and set their absolute positions
    this->pHdmi2_3 = new QPushButton("HDMI3", this);
    this->pHdmi2_3->setStyleSheet(buttonStylesheet);
    this->pHdmi2_3->move(screenSize.width() / 16 * 14, screenSize.height() / 108 * 22); // Set the position of the third button, x-coordinate is 100, y-coordinate is 100
    this->pHdmi2_3->resize(screenSize.width() / 15, screenSize.height() / 108 * 4);

    this->pExit2_1 = new QPushButton("EXIT", this);
    this->pExit2_1->setStyleSheet(buttonStylesheet);
    this->pExit2_1->move(screenSize.width() / 16 * 14, screenSize.height() / 108 * 27); // Set the position of the fourth button, x-coordinate is 300, y-coordinate is 200
    this->pExit2_1->resize(screenSize.width() / 15, screenSize.height() / 108 * 4);
    this->pExit2_1->setEnabled(false);

    // Create an undo button
    this->pUndoButton = new QPushButton("Undo", this);
    connect(this->pUndoButton, &QPushButton::clicked, this->pDrawingBoard, &Whiteboard::undoLastStroke);

    // Adjust the position and size of the button
    this->pUndoButton->setGeometry(screenSize.width() / 4 * 2, screenSize.height() / 10 * 9, 80, 30);
    this->pUndoButton->setStyleSheet(buttonStylesheet);
    this->pUndoButton->move(screenSize.width() / 4 * 1, screenSize.height() / 10 * 9); // Set the position of the first button, x-coordinate is 100, y-coordinate is 100
    this->pUndoButton->resize(screenSize.width() / 16, screenSize.height() / 108 * 3);

    // Create a clear button
    this->pClearButton = new QPushButton("Clear", this);
    connect(this->pClearButton, &QPushButton::clicked, this->pDrawingBoard, &Whiteboard::clearLastStroke);

    // Adjust the position and size of the button
    this->pClearButton->setGeometry(screenSize.width() / 4 * 2, screenSize.height() / 10 * 9, 80, 30);
    this->pClearButton->setStyleSheet(buttonStylesheet);
    this->pClearButton->move(screenSize.width() / 4 * 2 + 100, screenSize.height() / 10 * 9); // Set the position of the first button, x-coordinate is 100, y-coordinate is 100
    this->pClearButton->resize(screenSize.width() / 16, screenSize.height() / 108 * 3);

    // Connect the click events of the buttons to the corresponding functions
    connect(pHdmi2_1, &QPushButton::clicked, this, &Page1::Selet_HDMI1);
    connect(pHdmi2_2, &QPushButton::clicked, this, &Page1::Selet_HDMI2);
    connect(pHdmi2_3, &QPushButton::clicked, this, &Page1::Selet_HDMI3);
    connect(pExit2_1, &QPushButton::clicked, this, &Page1::Selet_HDMIQ);
    //connect(pFullScreen2_1, &QPushButton::clicked, this, &Page1::FullScreen);
}

static int SetOsdBlankStatus(const char *path, const char *cmd)
{
    return WriteSysfs(path, cmd);
}

static int DisplayInit()
{
    WriteSysfs("/sys/class/graphics/fb0/osd_display_debug", 1);
    WriteSysfs("/sys/class/graphics/fb0/blank", 0);
    WriteSysfs("/sys/kernel/debug/dri/0/vpu/blank", 0);
    return 0;
}

static int Draw_display()
{
    int x = 0; // Take the top-left coordinate as the origin and x as the width from the origin
    int y = 0; // Take the top-left coordinate as the origin and y as the height from the origin
    int w = 1920; // The actual display width in xy coordinates
    int h = 1080; // The actual display height in xy coordinates

    return 0;
}

void Page1::Selet_HDMI1()
{
    pRxtest = new QtRx();
    pRxtest->EdidVer = 14;

    DisplayInit();
    Draw_display();
    pRxtest->SendCmd("stop");
    pRxtest->CurrentSource = SOURCE_HDMI1;

    pRxtest->SendCmd("start");

    // Update button status
    qobject_cast<QPushButton *>(sender())->setEnabled(false);
    pExit2_1->setEnabled(true);
    pHdmi2_2->setEnabled(true);
    pHdmi2_3->setEnabled(true);
}

void Page1::Selet_HDMI2()
{
    pRxtest = new QtRx();
    DisplayInit();
    Draw_display();
    pRxtest->SendCmd("stop");
    pRxtest->CurrentSource = SOURCE_HDMI2;
    pRxtest->SendCmd("start");

    // Update button status
    qobject_cast<QPushButton *>(sender())->setEnabled(false);
    pExit2_1->setEnabled(true);
    pHdmi2_1->setEnabled(true);
    pHdmi2_3->setEnabled(true);
}

void Page1::Selet_HDMI3()
{
    pRxtest = new QtRx();
    DisplayInit();
    Draw_display();
    pRxtest->SendCmd("stop");
    pRxtest->CurrentSource = SOURCE_HDMI3;
    pRxtest->SendCmd("start");

    // Update button status
    qobject_cast<QPushButton *>(sender())->setEnabled(false);
    pExit2_1->setEnabled(true);
    pHdmi2_1->setEnabled(true);
    pHdmi2_2->setEnabled(true);
}

void Page1::Selet_HDMIQ()
{
    pRxtest->SendCmd("stop");

    // Update button status
    qobject_cast<QPushButton *>(sender())->setEnabled(false);
    pHdmi2_1->setEnabled(true);
    pHdmi2_2->setEnabled(true);
    pHdmi2_3->setEnabled(true);
}

void Page1::FullScreen()
{
    SetOsdBlankStatus("/sys/class/video/axis", "0 0 0 0");
    WriteSysfs("/sys/kernel/debug/dri/0/vpu/blank", 1);

    // Update button status
    fflush(stdout);
}
