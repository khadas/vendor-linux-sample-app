#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include <QtWidgets>

class Whiteboard : public QWidget {
    Q_OBJECT

public:
    Whiteboard(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_StaticContents);
        drawing = false;
        image = QImage(size(), QImage::Format_RGB32);
        image.fill(Qt::white);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        // painter.drawImage(rect(), image);
        // Set the background to transparent
        painter.fillRect(event->rect(), Qt::transparent);

        // Draw saved strokes
        painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        for (const QList<QPoint>& stroke : strokes) {
            for (int i = 1; i < stroke.size(); ++i) {
                painter.drawLine(stroke[i - 1], stroke[i]);
            }
        }

        for (int i = 1; i < currentStroke.size(); ++i) {
            painter.drawLine(currentStroke[i - 1], currentStroke[i]);
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "enter mousePressEvent";
        if (event->button() == Qt::LeftButton) {
            currentStroke.clear();
            currentStroke << event->pos();
            drawing = true;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        qDebug() << "enter mouseMoveEvent";
        if (drawing) {
            currentStroke << event->pos();
            update(); // Trigger a redraw to display the new stroke
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        qDebug() << "enter mouseReleaseEvent";
        if (event->button() == Qt::LeftButton && drawing) {
            strokes.append(currentStroke);
            drawing = false;
        }
    }

private:
    bool drawing;
    QImage image;
    QList<QList<QPoint>> strokes; // Save multiple drawing trajectories
    QList<QPoint> currentStroke;  // Current drawing stroke

public slots:
    void undoLastStroke() { // Add a boolean parameter to match the signal
        currentStroke.clear();
        if (!strokes.isEmpty()) {
            strokes.removeLast();
            update(); // Redraw to remove the last stroke
        }
    }

    void clearLastStroke() {
        strokes.clear(); // Clear all strokes
        currentStroke.clear();
        update(); // Force a redraw of the Whiteboard, clearing all drawing content
    }
};

#endif
