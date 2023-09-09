#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPixmap>
#include "zoomui.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
private:
    QPixmap mainimg;

public:
    MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setupUi(this);
        connect(endButton, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()));
    }

public slots:
    void processImage(const QImage &img);
};
#endif