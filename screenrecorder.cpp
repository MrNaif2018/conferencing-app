#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <string>
#include "opencv2/opencv.hpp"
#include <uvgrtp/lib.hh>

#include "screenrecorder.h"
#include "imageutil.h"
#include "cvmatandqimage.h"
#include "config.h"
#include <vector>

using namespace cv;

void ScreenRecorder::run()
{
    QRect rect = QApplication::desktop()->screenGeometry();
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(server);
    int flags = RCE_FRAGMENT_GENERIC | RCE_SEND_ONLY;
    uvgrtp::media_stream *hevc = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    std::vector<uint8_t> encoded;
    uint32_t frame_counter = 0;
    qDebug() << "before";
    qDebug() << sizeof(frame_counter);
    if (hevc)
    {
        Mat image, send;
        while (!QThread::currentThread()->isInterruptionRequested())
        {
            QPixmap pixmap = imageutil::takeScreenShot(rect);
            image = QtOcv::image2Mat(pixmap.toImage());
            resize(image, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            std::vector<int> compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(ENCODE_QUALITY);
            imencode(".jpg", send, encoded, compression_params);
            int payload_len = encoded.size();
            int current_seq = 0;
            auto header_frame = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) + 2 * sizeof(int)]);
            memcpy(header_frame.get(), &frame_counter, sizeof(frame_counter));
            memcpy(header_frame.get() + sizeof(frame_counter), &current_seq, sizeof(current_seq));
            memcpy(header_frame.get() + sizeof(frame_counter) + sizeof(current_seq), &payload_len, sizeof(payload_len));
            hevc->push_frame(header_frame.get(), sizeof(uint32_t) + 2 * sizeof(int), RTP_NO_FLAGS);
            current_seq++;
            std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL));
            int total_pack = 1 + (payload_len - 1) / PACK_SIZE;
            // qDebug() << "here we go " << payload_len;
            for (int i = 0; i < total_pack; i++)
            {
                // qDebug() << "Sending packet " << i << " of " << total_pack << " " << PACK_SIZE << " " << payload_len;
                int to_send = min<int>(PACK_SIZE, payload_len - i * PACK_SIZE);
                // qDebug() << "SENDING " << frame_counter << " " << i + 1 << " " << total_pack << " " << payload_len << " " << to_send << " " << i * PACK_SIZE;
                auto frame = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) + sizeof(int) + to_send]);
                memcpy(frame.get(), &frame_counter, sizeof(frame_counter));
                memcpy(frame.get() + sizeof(frame_counter), &current_seq, sizeof(current_seq));
                memcpy(frame.get() + sizeof(frame_counter) + sizeof(current_seq), encoded.data() + i * PACK_SIZE, to_send);
                hevc->push_frame(frame.get(), sizeof(uint32_t) + sizeof(int) + to_send, RTP_NO_FLAGS);
                current_seq++;
                std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL));
            }
            frame_counter++;
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