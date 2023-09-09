#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPixmap>
#include <QAudioInput>
#include <QUdpSocket>
#include "zoomui.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
private:
    QPixmap mainimg;
    QAudioInput *audio_input;
    QUdpSocket *audio_socket;
    bool mic_enabled;

public:
    MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent), mic_enabled(true)
    {
        setupUi(this);
        connect(endButton, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()));
        connect(micButton, SIGNAL(clicked()), this, SLOT(toggleMic()));
    }
    void init_audio_input(char *server);
    void start_audio()
    {
        audio_input->start(audio_socket);
    }
    void stop_audio()
    {
        audio_input->stop();
    }

public slots:
    void processImage(const QImage &img);
    void toggleMic()
    {
        mic_enabled = !mic_enabled;
        if (mic_enabled)
            start_audio();
        else
            stop_audio();
    }
};
#endif