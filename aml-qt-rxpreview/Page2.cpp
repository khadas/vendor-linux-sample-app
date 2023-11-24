// ImageDisplayThread.cpp
#include <QtWidgets>
#include <QLabel>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include "Page2.h"

Page2::Page2(QWidget *parent) {

    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();

    // Get the size of the primary screen (assuming it's the 4K screen)
    QSize screenSize = screen->size();

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

    // "Page 2 Content" label
    this->pLabelPage2 = new QLabel("Whiteboard", this);
    pLabelPage2->move(0, 0); // Set the position of the label, x-coordinate is 100, y-coordinate is 50
    // Set the font size of the label
    this->pLabelPage2->setFont(QFont("Arial", screenSize.width() / 96 )); // Use Arial font with font size in pixels
    // Set the size of the label to fit the text size and add extra padding
    this->pLabelPage2->setFixedSize(this->pLabelPage2->fontMetrics().boundingRect("Whiteboard").size() + QSize(20, 10));

    this->pdrawingBoard = new Whiteboard(this);
    this->pdrawingBoard->resize(screenSize.width(), screenSize.height()); // Scale the photo to fill the page while maintaining aspect ratio
    this->pdrawingBoard->move(0, 0); // Set the position of the photo

    // Create Undo button
    this->pUndoButton = new QPushButton("Undo", this);
    connect(this->pUndoButton, &QPushButton::clicked, this->pdrawingBoard, &Whiteboard::undoLastStroke);

    // Adjust the position and size of the button
    this->pUndoButton->setGeometry(screenSize.width() / 4 * 2,  screenSize.height() / 10 * 9, 80, 30);
    this->pUndoButton->setStyleSheet(buttonStylesheet);
    this->pUndoButton->move(screenSize.width() / 4 * 1,  screenSize.height() / 10 * 9); // Set the position of the first button, x-coordinate is 100, y-coordinate is 100
    this->pUndoButton->resize(screenSize.width() / 16 ,screenSize.height() / 108 * 3);

    // Create Clear button
    this->pClearButton = new QPushButton("Clear", this);
    connect(this->pClearButton, &QPushButton::clicked, this->pdrawingBoard, &Whiteboard::clearLastStroke);

    // Adjust the position and size of the button
    this->pClearButton->setGeometry(screenSize.width() / 4 * 2,  screenSize.height() / 10 * 9, 80, 30);
    this->pClearButton->setStyleSheet(buttonStylesheet);
    this->pClearButton->move(screenSize.width() / 4 * 2 + 100,  screenSize.height() / 10 * 9); // Set the position of the second button, x-coordinate is 100, y-coordinate is 100
    this->pClearButton->resize(screenSize.width() / 16 ,screenSize.height() / 108 * 3);

}

Page2::~Page2()
{
    // Release dynamically allocated resources in the destructor

    // Delete member variable objects

    // Other member variables do not need to be manually deleted as they are automatic objects and will be destroyed automatically when the function exits
}
