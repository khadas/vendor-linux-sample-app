#include "mainwindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <fstream>

// 在类外部定义静态成员变量
//QLabel* MainWindow::videoWidget2 = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

     move(0, 0);
	// Apply a mature and subdued stylesheet to buttons
	QString buttonStylesheet = "QPushButton {"
							   "	background-color: #34495e;"
							   "	border: none;"
							   "	color: white;"
							   "	padding: 10px 20px;"
							   "	text-align: center;"
							   "	text-decoration: none;"
							   "	display: inline-block;"
							   "	font-size: 30px;"
							   "	border-radius: 8px;"
							   "	box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1);"
							   "}"
							   "QPushButton:hover {"
							   "	background-color: #2c3e50;"
							   "	cursor: pointer;"
							   "}"
							   "QPushButton:pressed {"
							   "	background-color: #273746;"
							   "	box-shadow: inset 0px 2px 4px rgba(0, 0, 0, 0.1);"
							   "}";


	QString backgroundStylesheet = "background-color: #c0c0c0;";

    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();

    // Get the size of the primary screen (assuming it's the 4K screen)
    QSize screenSize = screen->size();

    // Set the window size to match the screen size
    setFixedSize(screenSize.width(), screenSize.height());

    this->stackedWidget = new QStackedWidget(this);

    this->page1 = new Page1(this);
    this->page2 = new Page2(this);
    this->page3 = new Page3(this);
    this->page4 = new QWidget(this);

    this->stackedWidget->addWidget(page1);
    this->stackedWidget->addWidget(page2);
    this->stackedWidget->addWidget(page3);
    this->stackedWidget->addWidget(page4);


    page4->setLayout(new QVBoxLayout());
    page4->layout()->addWidget(new QLabel("Page 4 Content"));

    setCentralWidget(stackedWidget);

    // Create the bottom buttons
    QPushButton *buttonPage1 = new QPushButton("Page 1", this);
    QPushButton *buttonPage2 = new QPushButton("Page 2", this);
    QPushButton *buttonPage3 = new QPushButton("Page 3", this);
    QPushButton *buttonPage4 = new QPushButton("Page 4", this);


	buttonPage1->setStyleSheet(buttonStylesheet);
	buttonPage2->setStyleSheet(buttonStylesheet);
	buttonPage3->setStyleSheet(buttonStylesheet);
	buttonPage4->setStyleSheet(buttonStylesheet);

    // Connect the buttons to the respective slots
    connect(buttonPage1, &QPushButton::clicked, this, &MainWindow::onButtonPage1Clicked);
    connect(buttonPage2, &QPushButton::clicked, this, &MainWindow::onButtonPage2Clicked);
    connect(buttonPage3, &QPushButton::clicked, this, &MainWindow::onButtonPage3Clicked);
    connect(buttonPage4, &QPushButton::clicked, this, &MainWindow::onButtonPage4Clicked);

    // Add the buttons to a horizontal layout
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(buttonPage1);
    bottomLayout->addWidget(buttonPage2);
    bottomLayout->addWidget(buttonPage3);
    bottomLayout->addWidget(buttonPage4);

    // Create a vertical layout for the central widget
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(bottomLayout);
    mainLayout->addWidget(stackedWidget);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::onButtonPage1Clicked()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::onButtonPage2Clicked()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onButtonPage3Clicked()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::onButtonPage4Clicked()
{
    stackedWidget->setCurrentIndex(3);
}




