#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPixmap>
#include <QAudioInput>
#include <QUdpSocket>

#include "zoomui.h"
#include "udpplayer.h"

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
        connect(micButton, SIGNAL(clicked()), this, SLOT(toggleMic()));
    }
    void init_audio_input(char *server)
    {
        QAudioFormat format = getAudioFormat();
        audio_input = new QAudioInput(format);
        audio_socket = new QUdpSocket();
        audio_socket->connectToHost(server, AUDIO_UDP_PORT);
        audio_socket->waitForConnected();
        start_audio();
    }
    void start_audio()
    {
        audio_input->start(audio_socket);
    }
    void stop_audio()
    {
        audio_input->stop();
    }
    void deinit_audio_input()
    {
        stop_audio();
        delete audio_socket;
        delete audio_input;
    }

public slots:
    void processImage(const QImage &img)
    {
        imgpix = QPixmap::fromImage(img);
        pixmap->setPixmap(imgpix);
    }
    void toggleMic()
    {
        mic_enabled = !mic_enabled;
        if (mic_enabled)
            start_audio();
        else
            stop_audio();
    }
    void beforeStopAll()
    {
        emit stopAll();
    }
signals:
    void stopAll();
};
#endif