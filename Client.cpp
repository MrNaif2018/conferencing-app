#include "PracticalSocket.h" // For UDPSocket and SocketException
#include <iostream>          // For cout and cerr
#include <cstdlib>           // For atoi()
#include <QPixmap>
#include <QImage>
#include <QtConcurrent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QCursor>
#include <QDebug>
#include "cvmatandqimage.h"
#include "udpplayer.h"
#include <QAudioFormat>
#include <QAudioInput>
#include <QUdpSocket>
#include "workerthread.h"
#include "audioworker.h"

using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"
#include "imageutil.h"

class TracingUDPSocket : public QUdpSocket
{
public:
    TracingUDPSocket(QObject *parent = nullptr) : QUdpSocket(parent) {}
    qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port)
    {
        qDebug() << "Writing datagram to " << host << ":" << port;
        return QUdpSocket::writeDatagram(datagram, host, port);
    }
    qint64 writeData(const char *data, qint64 len)
    {
        qDebug() << "Writing data"
                 << " " << data << " " << len;
        return QUdpSocket::writeData(data, len);
    }
};

void AudioWorkerThread::run()
{
    QAudioFormat format = getAudioFormat();
    QAudioInput *input = new QAudioInput(format);
    TracingUDPSocket *socket = new TracingUDPSocket();
    socket->open(QIODevice::WriteOnly);
    qDebug() << "Binding to port " << AUDIO_UDP_PORT;
    socket->connectToHost(argv[1], AUDIO_UDP_PORT);
    qDebug() << "in progress connect";
    socket->waitForConnected();
    qDebug() << "established";
    input->start(socket);
    qDebug() << "done";
    while (1)
    {
        QThread::msleep(1000);
    }
}

void record_audio_task(string server)
{
}

void MyThread::run()
{
    QRect rect = QApplication::desktop()->screenGeometry();
    string servAddress = argv[1];
    unsigned short servPort = Socket::resolveService(argv[2], "udp");
    qDebug() << argv[2];
    try
    {
        UDPSocket sock;
        int jpegqual = ENCODE_QUALITY;
        Mat frame, send;
        vector<uchar> encoded;
        // VideoCapture cap(0); // Grab the camera
        clock_t last_cycle = clock();
        while (1)
        {
            QPixmap pixmap = imageutil::takeScreenShot(rect);
            cv::Mat frame = QtOcv::image2Mat(pixmap.toImage());
            if (frame.size().width == 0)
                continue;
            qDebug() << frame.size().width << " " << frame.size().height;
            resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            vector<int> compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegqual);
            imencode(".jpg", send, encoded, compression_params);
            int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;
            int ibuf[1];
            ibuf[0] = total_pack;
            qDebug() << "prepare send";
            sock.sendTo(ibuf, sizeof(int), servAddress, servPort);
            for (int i = 0; i < total_pack; i++)
                sock.sendTo(&encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
            QThread::msleep(FRAME_INTERVAL);
            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double)CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;
            cout << next_cycle - last_cycle;
            last_cycle = next_cycle;
        }
    }
    catch (SocketException &e)
    {
        cerr << e.what() << endl;
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (argc != 3)
    { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port>\n";
        exit(1);
    }
    new UDPPlayer();
    start_audio_input(argv);
    MyThread thread(argv);
    thread.start();
    // AudioWorkerThread audioThread(argv);
    // audioThread.start();
    return app.exec();
}
