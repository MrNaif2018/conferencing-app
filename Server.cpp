#include <iostream> // For cout and cerr
#include <cstdlib>  // For atoi()
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
#include <map>
#include <set>

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
    std::cout << "Starting uvgRTP RTP receive hook example" << std::endl;
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session("0.0.0.0");
    int flags = RCE_FRAGMENT_GENERIC | RCE_RECEIVE_ONLY;
    uvgrtp::media_stream *receiver = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    if (receiver)
    {
        // std::cout << "Start receiving frames" << std::endl;
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
            // qDebug() << "GOT " << chunks[current_frame].begin()->seq;
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
                    // qDebug() << "in loop " << current_frame << " " << it->seq;
                    // qDebug() << "frame: " << current_frame << " buff_size: " << buffer_size << " size: " << it->size;
                    memcpy(buffer + offset, it->data, it->size);
                    offset += it->size;
                }
                // qDebug() << "got offset " << offset << " of frame " << current_frame << " with buf " << buffer_size;
                if (offset == buffer_size)
                {
                    // qDebug() << "GOT CORRECT DATA " << offset << " frame: " << current_frame;
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

void MainWindow::processImage(const QImage &img)
{
    // qDebug() << "GOT";
    imgpix = QPixmap::fromImage(img);
    pixmap->setPixmap(imgpix);
}

void MainWindow::init_audio_input(char *server)
{
    QAudioFormat format = getAudioFormat();
    audio_input = new QAudioInput(format);
    audio_socket = new QUdpSocket();
    // audio_socket->open(QIODevice::WriteOnly);
    qDebug() << "Binding to port " << AUDIO_UDP_PORT;
    audio_socket->connectToHost(server, AUDIO_UDP_PORT);
    audio_socket->waitForConnected();
    start_audio();
}

void play_audio_task()
{
    new UDPPlayer();
}

int main(int argc, char **argv)
{
    if (argc != 2)
    { // Test for correct number of parameters
        std::cerr << "Usage: " << argv[0] << " <Server>" << std::endl;
        exit(1);
    }
    QApplication app(argc, argv);
    new UDPPlayer();
    ScreenRecorder recorder(argv[1]);
    recorder.start();
    MainWindow window;
    window.init_audio_input(argv[1]);
    window.show();
    MyThread thread;
    QObject::connect(&thread, SIGNAL(signalGUI(const QImage &)), &window, SLOT(processImage(const QImage &)));
    thread.start();
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), &thread, SLOT(terminateThread()));
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()), &recorder, SLOT(terminateThread()));
    // QtConcurrent::run(play_audio_task);
    return app.exec();
}
