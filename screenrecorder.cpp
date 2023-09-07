#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <string>
#include "opencv2/opencv.hpp"

#include "screenrecorder.h"
#include "PracticalSocket.h"
#include "imageutil.h"
#include "cvmatandqimage.h"
#include "config.h"

using namespace cv;

void ScreenRecorder::run()
{
    QRect rect = QApplication::desktop()->screenGeometry();
    string servAddress = server;
    unsigned short servPort = IMAGE_UDP_PORT;
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