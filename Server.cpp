#include <iostream>
#include <cstdlib>
#include <map>
#include <set>

#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QFile>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QSettings>

#include <uvgrtp/lib.hh>
#include "opencv2/opencv.hpp"
using namespace cv;

#include "udpplayer.h"
#include "screenrecorder.h"
#include "config.h"
#include "zoomui.h"
#include "cvmatandqimage.h"
#include "workerthread.h"
#include "mainwindow.h"
#include "startwindow.h"
#include "settingswindow.h"

constexpr int RECEIVER_WAIT_TIME_MS = 10 * 1000;

struct FrameData
{
    int frame_num;
    int buffer_size;
    FrameData() : frame_num(-1), buffer_size(0) {}
    FrameData(int frame_num, int buffer_size) : frame_num(frame_num), buffer_size(buffer_size) {}
};

struct FrameChunk
{
    int seq;
    int size;
    uint8_t *data;
    FrameChunk() : seq(-1), size(0), data(nullptr) {}
    FrameChunk(int seq, int size, uint8_t *data) : seq(seq), size(size), data(data) {}
    bool operator<(const FrameChunk &other) const
    {
        return seq < other.seq;
    }
    FrameChunk(const FrameChunk &other) : seq(other.seq), size(other.size)
    {
        data = new uint8_t[size];
        memcpy(data, other.data, size);
    }
    ~FrameChunk()
    {
        delete[] data;
    }
};

void MyThread::run()
{
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session("0.0.0.0");
    int flags = RCE_FRAGMENT_GENERIC | RCE_RECEIVE_ONLY;
    uvgrtp::media_stream *receiver = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    if (receiver)
    {
        std::map<uint32_t, FrameData> frames;
        std::map<uint32_t, std::set<FrameChunk>> chunks;
        while (!QThread::currentThread()->isInterruptionRequested())
        {
            uvgrtp::frame::rtp_frame *frame = receiver->pull_frame(RECEIVER_WAIT_TIME_MS);
            if (!frame)
                break;
            uint32_t current_frame;
            int current_seq;
            memcpy(&current_frame, frame->payload, sizeof(uint32_t));
            memcpy(&current_seq, frame->payload + sizeof(uint32_t), sizeof(int));
            size_t real_len = frame->payload_len - sizeof(uint32_t) - sizeof(int);
            uint8_t *data = new uint8_t[real_len];
            memcpy(data, frame->payload + sizeof(uint32_t) + sizeof(int), real_len);
            chunks[current_frame].insert(FrameChunk(current_seq, real_len, data));
            if (chunks[current_frame].begin()->seq == 0) // received header
            {
                int buffer_size;
                memcpy(&buffer_size, chunks[current_frame].begin()->data, sizeof(int));
                frames[current_frame] = FrameData(current_frame, buffer_size);
                chunks[current_frame].erase(chunks[current_frame].begin());
            }
            if (frames.count(current_frame))
            {
                int offset = 0;
                int buffer_size = frames[current_frame].buffer_size;
                uint8_t *buffer = new uint8_t[buffer_size];
                for (auto it = chunks[current_frame].begin(); it != chunks[current_frame].end(); it++)
                {
                    memcpy(buffer + offset, it->data, it->size);
                    offset += it->size;
                }
                if (offset == buffer_size)
                {
                    Mat rawData = Mat(1, buffer_size, CV_8UC1, buffer);
                    Mat cvimg = imdecode(rawData, IMREAD_COLOR);
                    if (cvimg.size().width == 0)
                    {
                        std::cerr << "decode failure!" << std::endl;
                        continue;
                    }
                    resize(cvimg, cvimg, Size(1278, 638), 0, 0, INTER_LINEAR);
                    QImage image = QtOcv::mat2Image(cvimg);
                    emit signalGUI(image);
                    frames.erase(current_frame);
                    chunks.erase(current_frame);
                }
                delete[] buffer;
            }
            (void)uvgrtp::frame::dealloc_frame(frame);
        }
        sess->destroy_stream(receiver);
    }
    if (sess)
        ctx.destroy_session(sess);
}

class SessionManager
{
private:
    UDPPlayer *player;
    ScreenRecorder *recorder;
    MyThread *listen_thread;
    MainWindow *window;
    StartWindow &startWindow;
    SettingsWindow *settingsWindow;
    std::string ConnectServer;

public:
    SessionManager(StartWindow &startWindow) : startWindow(startWindow)
    {
        QObject::connect(startWindow.settingsButton, &QPushButton::clicked, [&]()
                         {
            settingsWindow = new SettingsWindow();
            settingsWindow->setFixedSize(settingsWindow->width(), settingsWindow->height());
            QObject::connect(settingsWindow->saveButton, &QPushButton::clicked, [&]()
                             {
                settingsWindow->saveSettings();
                settingsWindow->close();
                settingsWindow->deleteLater(); });
            settingsWindow->show(); });
    }
    void start()
    {
        startWindow.hide();
        player = new UDPPlayer();
        QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
        int pack_size = settings.value("pack_size", PACK_SIZE).toInt();
        int frame_interval = (1000 / settings.value("fps", FPS).toInt());
        int quality = settings.value("quality", ENCODE_QUALITY).toInt();
        recorder = new ScreenRecorder((char *)ConnectServer.c_str(), pack_size, frame_interval, quality);
        recorder->start();
        window = new MainWindow();
        window->setFixedSize(window->width(), window->height());
        QObject::connect(window->endButton, &QPushButton::clicked, [&](bool)
                         {
            stop();
            startWindow.show(); });
        window->init_audio_input((char *)ConnectServer.c_str());
        window->show();
        listen_thread = new MyThread();
        QObject::connect(listen_thread, SIGNAL(signalGUI(const QImage &)), window, SLOT(processImage(const QImage &)));
        QObject::connect(listen_thread, &QThread::finished, window, &MainWindow::beforeStopAll);
        QObject::connect(window, &MainWindow::stopAll, [&]()
                         { stop();
            startWindow.show(); });
        listen_thread->start();
        QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), listen_thread, SLOT(terminateThread()));
        QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), recorder, SLOT(terminateThread()));
    }
    void stop()
    {
        listen_thread->terminateThread();
        recorder->terminateThread();
        window->deinit_audio_input();
        listen_thread->deleteLater();
        recorder->deleteLater();
        player->deleteLater();
        window->deleteLater();
    }
    void connectButtonClicked()
    {
        QString ip = startWindow.ipLabel->text();
        if (ip.isEmpty() || QHostAddress(ip).isNull())
        {
            QMessageBox::warning(&startWindow, "Error", "Please enter an IP address");
            return;
        }
        ConnectServer = ip.toLocal8Bit().data();
        start();
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    StartWindow startWindow;
    startWindow.setFixedSize(startWindow.width(), startWindow.height());
    SessionManager manager(startWindow);
    QObject::connect(startWindow.connectButton, &QPushButton::clicked, [&]()
                     { manager.connectButtonClicked(); });
    startWindow.show();
    return app.exec();
}
