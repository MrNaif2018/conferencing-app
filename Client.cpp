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

#ifdef Q_OS_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#else // Q_OS_WINDOWs
#include <Windows.h>
#include <wingdi.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#include <QtWinExtras>
#endif

namespace imageutil
{

#ifdef Q_OS_LINUX

    /* WebCore/plugins/qt/QtX11ImageConversion.cpp */
    QImage qimageFromXImage(XImage *xi)
    {
        QImage::Format format = QImage::Format_ARGB32_Premultiplied;
        if (xi->depth == 24)
            format = QImage::Format_RGB32;
        else if (xi->depth == 16)
            format = QImage::Format_RGB16;

        QImage image = QImage(reinterpret_cast<uchar *>(xi->data), xi->width, xi->height, xi->bytes_per_line, format).copy();

        // we may have to swap the byte order
        if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && xi->byte_order == MSBFirst) || (QSysInfo::ByteOrder == QSysInfo::BigEndian && xi->byte_order == LSBFirst))
        {

            for (int i = 0; i < image.height(); i++)
            {
                if (xi->depth == 16)
                {
                    ushort *p = reinterpret_cast<ushort *>(image.scanLine(i));
                    ushort *end = p + image.width();
                    while (p < end)
                    {
                        *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                        p++;
                    }
                }
                else
                {
                    uint *p = reinterpret_cast<uint *>(image.scanLine(i));
                    uint *end = p + image.width();
                    while (p < end)
                    {
                        *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000) | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                        p++;
                    }
                }
            }
        }

        // fix-up alpha channel
        if (format == QImage::Format_RGB32)
        {
            QRgb *p = reinterpret_cast<QRgb *>(image.bits());
            for (int y = 0; y < xi->height; ++y)
            {
                for (int x = 0; x < xi->width; ++x)
                    p[x] |= 0xff000000;
                p += xi->bytes_per_line / 4;
            }
        }

        return image;
    }

#endif // Q_OS_LINUX

    QPixmap takeScreenShot(const QRect &area)
    {
        QRect screen;  /* interested display area */
        QImage qimage; /* result image */

#ifdef Q_OS_LINUX
        QPoint cursorPos;

        Display *display = XOpenDisplay(nullptr);
        Window root = DefaultRootWindow(display);

        XWindowAttributes gwa;
        XGetWindowAttributes(display, root, &gwa);

        const auto goodArea = QRect(0, 0, gwa.width, gwa.height).contains(area);
        if (!goodArea)
        {
            screen = QRect(0, 0, gwa.width, gwa.height);
            cursorPos = QCursor::pos();
        }
        else
        {
            screen = area;
            cursorPos = QCursor::pos() - screen.topLeft();
        }

        XImage *image = XGetImage(display, root, screen.x(), screen.y(), screen.width(), screen.height(), AllPlanes, ZPixmap);
        assert(nullptr != image);

        qimage = qimageFromXImage(image);

        /* draw mouse cursor into QImage
         * https://msnkambule.wordpress.com/2010/04/09/capturing-a-screenshot-showing-mouse-cursor-in-kde/
         * https://github.com/rprichard/x11-canvas-screencast/blob/master/CursorX11.cpp#L31
         * */
        {
            XFixesCursorImage *cursor = XFixesGetCursorImage(display);
            cursorPos -= QPoint(cursor->xhot, cursor->yhot);
            std::vector<uint32_t> pixels(cursor->width * cursor->height);
            for (size_t i = 0; i < pixels.size(); ++i)
                pixels[i] = cursor->pixels[i];
            QImage cursorImage((uchar *)(pixels.data()), cursor->width, cursor->height, QImage::Format_ARGB32_Premultiplied);
            QPainter painter(&qimage);
            painter.drawImage(cursorPos, cursorImage);
            XFree(cursor);
        }

        XDestroyImage(image);
        XDestroyWindow(display, root);
        XCloseDisplay(display);

#elif defined(Q_OS_WINDOWS)
        HWND hwnd = GetDesktopWindow();
        HDC hdc = GetWindowDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdc);

        RECT rect = {0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES)};
        const auto goodArea = QRect(rect.left, rect.top, rect.right, rect.bottom).contains(area);
        if (!goodArea)
        {
            screen = QRect(rect.left, rect.top, rect.right, rect.bottom);
        }
        else
        {
            screen = area;
        }

        HBITMAP hbitmap(nullptr);
        hbitmap = CreateCompatibleBitmap(hdc, screen.width(), screen.height());
        SelectObject(hdcMem, hbitmap);
        BitBlt(hdcMem, 0, 0, screen.width(), screen.height(), hdc, screen.x(), screen.y(), SRCCOPY);

        /* draw mouse cursor into DC
         * https://stackoverflow.com/a/48925443/5446734
         * */
        CURSORINFO cursor = {sizeof(cursor)};
        if (GetCursorInfo(&cursor) && cursor.flags == CURSOR_SHOWING)
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            ICONINFO info = {sizeof(info)};
            GetIconInfo(cursor.hCursor, &info);
            const int x = (cursor.ptScreenPos.x - rect.left - rect.left - info.xHotspot) - screen.left();
            const int y = (cursor.ptScreenPos.y - rect.left - rect.left - info.yHotspot) - screen.top();
            BITMAP bmpCursor = {0};
            GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
            DrawIconEx(hdcMem, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
                       0, nullptr, DI_NORMAL);
        }

        qimage = QtWin::imageFromHBITMAP(hdc, hbitmap, screen.width(), screen.height());
#endif // Q_OS_LINUX

        return QPixmap::fromImage(qimage);
    }

} // namespace imageutil

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
    qDebug() << "HERE";
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
        qDebug() << "GG";
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
    QAudioFormat format = getAudioFormat();
    QAudioInput *input = new QAudioInput(format);
    TracingUDPSocket *socket = new TracingUDPSocket();
    socket->open(QIODevice::WriteOnly);
    qDebug() << "Binding to port " << AUDIO_UDP_PORT;
    // socket->connectToHost(argv[1], AUDIO_UDP_PORT);
    socket->connectToHost(argv[1], AUDIO_UDP_PORT);
    qDebug() << "in progress connect";
    socket->waitForConnected();
    qDebug() << "established";
    input->start(socket);
    qDebug() << "done";
    MyThread thread(argv);
    thread.start();
    // AudioWorkerThread audioThread(argv);
    // audioThread.start();
    return app.exec();
}
