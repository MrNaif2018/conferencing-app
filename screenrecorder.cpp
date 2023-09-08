#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <string>
#include "opencv2/opencv.hpp"
#include <uvgrtp/lib.hh>

#include "screenrecorder.h"
#include "PracticalSocket.h"
#include "imageutil.h"
#include "cvmatandqimage.h"
#include "config.h"

using namespace cv;

void ScreenRecorder::run()
{
    QRect rect = QApplication::desktop()->screenGeometry();
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(server);
    int flags = RCE_FRAGMENT_GENERIC | RCE_SEND_ONLY;
    uvgrtp::media_stream *hevc = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    vector<uint8_t> encoded;
    if (hevc)
    {
        Mat image, send;
        while (!QThread::currentThread()->isInterruptionRequested())
        {
            QPixmap pixmap = imageutil::takeScreenShot(rect);
            image = QtOcv::image2Mat(pixmap.toImage());
            resize(image, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            vector<int> compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(ENCODE_QUALITY);
            imencode(".jpg", send, encoded, compression_params);
            auto frame = std::unique_ptr<uint8_t[]>(new uint8_t[encoded.size()]);
            memcpy(frame.get(), encoded.data(), encoded.size());
            // qDebug() << "ENC " << encoded.size();
            int payload_len = encoded.size();
            auto header_frame = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(int)]);
            memcpy(header_frame.get(), &payload_len, sizeof(payload_len));
            hevc->push_frame(header_frame.get(), sizeof(int), RTP_NO_FLAGS);
            std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL));
            int total_pack = 1 + (payload_len - 1) / PACK_SIZE;
            for (int i = 0; i < total_pack; i++)
            {
                // qDebug() << "Sending packet " << i << " of " << total_pack << " " << PACK_SIZE << " " << payload_len;
                hevc->push_frame(frame.get() + i * PACK_SIZE, PACK_SIZE, RTP_NO_FLAGS);
                std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL));
            }
            frame.reset();
            header_frame.reset();
            // if (hevc->push_frame(frame.get(), payload_len, RCE_FRAGMENT_GENERIC) != RTP_OK)
            // {
            //     std::cout << "Failed to send RTP frame!" << std::endl;
            // }
            std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL));
        }
        sess->destroy_stream(hevc);
    }
    if (sess)
    {
        /* Session must be destroyed manually */
        ctx.destroy_session(sess);
    }
}