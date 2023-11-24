// ImageDisplayThread.cpp
#include <QtWidgets>
#include <QLabel>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include "Page3.h"

Page3::Page3(QWidget *parent) {

    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();

    // Get the size of the primary screen (assuming it's the 4K screen)
    QSize screenSize = screen->size();

    // Add a photo to the first page
    QPixmap photo(":/my/photo/fleur-SE2zTdS1MNo-unsplash.jpg"); // Replace with the actual path to your photo
    this->plabelPhoto1_1 = new QLabel(this);
    this->plabelPhoto1_1->setPixmap(photo.scaled(screenSize.width(), screenSize.height())); // Scale the photo to fill the page while maintaining aspect ratio
    this->plabelPhoto1_1->setGeometry(0, 0, screenSize.width(), screenSize.height()); // Set the position and size of the photo

    // "Page 1 Content" label
    this->pLabelPage1 = new QLabel("brightness", this);
    this->pLabelPage1->move(0, 0); // Set the position of the label, x-coordinate is 100, y-coordinate is 50
    // Set the font size of the label
    this->pLabelPage1->setFont(QFont("Arial", screenSize.width() / 96 )); // Use Arial font with font size in pixels
    // Set the size of the label to fit the text size and add extra padding
    this->pLabelPage1->setFixedSize(this->pLabelPage1->fontMetrics().boundingRect("brightness").size() + QSize(20, 10));

    // Slider
    this->psliderPage1 = new QSlider(Qt::Horizontal, this); // Horizontal slider
    psliderPage1->resize(screenSize.width() * 3 / 5, 200); // Set the size of the slider, width is 300, height is 50
    psliderPage1->move(screenSize.width() * 1 / 5, screenSize.height() / 50); // Set the position of the slider, x-coordinate is 200, y-coordinate is 200
    psliderPage1->setMinimum(0); // Minimum value
    psliderPage1->setMaximum(255); // Maximum value, assuming the maximum brightness value is 19200
    psliderPage1->setValue(255); // Set the default brightness value
    // Connect the slider to the slot function
    connect(psliderPage1, &QSlider::valueChanged, this, &Page3::onSliderValueChanged);

}

// Function: Set screen brightness
bool Page3::onSliderValueChanged(int brightness)

{
    // Limit the brightness value within a reasonable range (typically between 0 and the maximum brightness value)
    const int minBrightness = 0;
    const int maxBrightness = 255; // Assume maximum brightness value is 255
    brightness = qBound(minBrightness, brightness, maxBrightness);

    // Convert brightness value to string
    QString brightnessStr = QString::number(brightness);

    // Open the brightness file and write the brightness value
    std::ofstream brightnessFile("/sys/class/backlight/aml-bl/brightness");
    if (!brightnessFile.is_open())
    {
        std::cerr << "Unable to open file /sys/class/backlight/aml-bl/brightness\n";
        return false;
    }

    brightnessFile << brightnessStr.toStdString();
    brightnessFile.close();
    return true;
}

Page3::~Page3()
{
    // Release dynamically allocated resources in the destructor

    // Delete member variable objects
    delete pLabelPage1;
    delete plabelPhoto1_1;
    delete psliderPage1;

    // Other member variables do not need to be manually deleted as they are automatic objects and will be destroyed automatically when the function exits
}
