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

#define BUF_LEN 65540 // Larger than maximum UDP packet size

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"
#include "zoomui.h"
#include "cvmatandqimage.h"
#include <QGraphicsPixmapItem>
#include "workerthread.h"
#include "mainwindow.h"

void MyThread::run()
{
    unsigned short servPort = IMAGE_UDP_PORT;
    try
    {
        UDPSocket sock(servPort);
        char buffer[BUF_LEN];      // Buffer for echo string
        int recvMsgSize;           // Size of received message
        string sourceAddress;      // Address of datagram source
        unsigned short sourcePort; // Port of datagram source
        clock_t last_cycle = clock();
        while (1)
        {
            // Block until we receive message from a client
            do
            {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
            } while (recvMsgSize > sizeof(int));
            int total_pack = ((int *)buffer)[0];
            cout << "expecting length of packs:" << total_pack << endl;
            char *longbuf = new char[PACK_SIZE * total_pack];
            for (int i = 0; i < total_pack; i++)
            {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != PACK_SIZE)
                {
                    cerr << "Received unexpected size pack:" << recvMsgSize << endl;
                    continue;
                }
                memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
            }
            cout << "Received packet from " << sourceAddress << ":" << sourcePort << endl;
            Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
            Mat frame = imdecode(rawData, IMREAD_COLOR);
            if (frame.size().width == 0)
            {
                cerr << "decode failure!" << endl;
                continue;
            }
            resize(frame, frame, Size(1280, 720), 0, 0, INTER_LINEAR);
            QImage image = QtOcv::mat2Image(frame);
            emit signalGUI(image);
            free(longbuf);
            QThread::msleep(1);
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

void MainWindow::processImage(const QImage &img)
{
    qDebug() << "GOT";
    imgpix = QPixmap::fromImage(img.copy());
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
    // QtConcurrent::run(play_audio_task);
    return app.exec();
}
