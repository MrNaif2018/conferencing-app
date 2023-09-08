#include "PracticalSocket.h" // For UDPSocket and SocketException
#include <iostream>          // For cout and cerr
#include <cstdlib>           // For atoi()
#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QFile>
#include <QtConcurrent>
#include "udpplayer.h"
#include "screenrecorder.h"
#include <uvgrtp/lib.hh>

#define BUF_LEN 65540 // Larger than maximum UDP packet size

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"
#include "zoomui.h"
#include "cvmatandqimage.h"
#include <QGraphicsPixmapItem>
#include "workerthread.h"
#include "mainwindow.h"

constexpr int RECEIVER_WAIT_TIME_MS = 10 * 1000;

void MyThread::run()
{
    std::cout << "Starting uvgRTP RTP receive hook example" << std::endl;
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session("0.0.0.0");
    int flags = RCE_FRAGMENT_GENERIC | RCE_RECEIVE_ONLY;
    uvgrtp::media_stream *receiver = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    if (receiver)
    {
        // std::cout << "Start receiving frames" << std::endl;
        bool receive_header = true;
        int buffer_len = 0;
        uint8_t *buffer = nullptr;
        int offset = 0;
        while (!QThread::currentThread()->isInterruptionRequested())
        {
            uvgrtp::frame::rtp_frame *frame = receiver->pull_frame(RECEIVER_WAIT_TIME_MS);
            if (!frame)
                break;
            if (receive_header)
            {
                // qDebug() << "payload_len header: " << frame->payload_len;
                if (frame->payload_len != sizeof(int))
                {
                    cerr << "header size error!" << endl;
                    continue;
                }
                memcpy(&buffer_len, frame->payload, sizeof(int));
                buffer = new uint8_t[buffer_len];
                receive_header = false;
            }
            else
            {
                // qDebug() << "payload_len main: " << frame->payload_len;
                int to_add = min<int>(buffer_len - offset, frame->payload_len);
                memcpy(buffer + offset, frame->payload, to_add);
                offset += to_add;
            }
            qDebug() << "offset: " << offset << " buffer_len: " << buffer_len;
            if (offset == buffer_len)
            {
                Mat rawData = Mat(1, buffer_len, CV_8UC1, buffer);
                Mat cvimg = imdecode(rawData, IMREAD_COLOR);
                if (cvimg.size().width == 0)
                {
                    cerr << "decode failure!" << endl;
                    continue;
                }
                resize(cvimg, cvimg, Size(1280, 720), 0, 0, INTER_LINEAR);
                QImage image = QtOcv::mat2Image(cvimg);
                emit signalGUI(image);
                offset = 0;
                receive_header = true;
                delete[] buffer;
                // qDebug() << buffer;
            }
            (void)uvgrtp::frame::dealloc_frame(frame);
        }
        sess->destroy_stream(receiver);
    }
    if (sess)
        ctx.destroy_session(sess);
}

void MainWindow::processImage(const QImage &img)
{
    // qDebug() << "GOT";
    imgpix = QPixmap::fromImage(img);
    pixmap->setPixmap(imgpix);
}

void play_audio_task()
{
    new UDPPlayer();
}

int main(int argc, char **argv)
{
    if (argc != 2)
    { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server>" << endl;
        exit(1);
    }
    QApplication app(argc, argv);
    new UDPPlayer();
    start_audio_input(argv[1]);
    ScreenRecorder recorder(argv[1]);
    recorder.start();
    MainWindow window;
    window.show();
    MyThread thread;
    QObject::connect(&thread, SIGNAL(signalGUI(const QImage &)), &window, SLOT(processImage(const QImage &)));
    thread.start();
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), &thread, SLOT(terminateThread()));
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), &recorder, SLOT(terminateThread()));
    // QtConcurrent::run(play_audio_task);
    return app.exec();
}
