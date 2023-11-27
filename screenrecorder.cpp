#include <string>
#include <vector>

#include <QApplication>
#include <QScreen>
#include <QDebug>

#include "opencv2/opencv.hpp"
#include <uvgrtp/lib.hh>

#include "screenrecorder.h"
#include "imageutil.h"
#include "cvmatandqimage.h"
#include "config.h"

using namespace cv;

void ScreenRecorder::run()
{
    QRect rect = QApplication::screens().at(0)->geometry();
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(server);
    int flags = RCE_FRAGMENT_GENERIC | RCE_SEND_ONLY;
    uvgrtp::media_stream *stream = sess->create_stream(IMAGE_UDP_PORT, RTP_FORMAT_GENERIC, flags);
    std::vector<uint8_t> encoded;
    uint32_t frame_counter = 0;
    if (stream)
    {
        Mat image, send;
        while (!QThread::currentThread()->isInterruptionRequested())
        {
            QPixmap pixmap = imageutil::takeScreenShot(rect);
            image = QtOcv::image2Mat(pixmap.toImage());
            resize(image, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            std::vector<int> compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(quality);
            imencode(".jpg", send, encoded, compression_params);
            int payload_len = encoded.size();
            int current_seq = 0;
            auto header_frame = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) + 2 * sizeof(int)]);
            memcpy(header_frame.get(), &frame_counter, sizeof(frame_counter));
            memcpy(header_frame.get() + sizeof(frame_counter), &current_seq, sizeof(current_seq));
            memcpy(header_frame.get() + sizeof(frame_counter) + sizeof(current_seq), &payload_len, sizeof(payload_len));
            stream->push_frame(header_frame.get(), sizeof(uint32_t) + 2 * sizeof(int), RTP_NO_FLAGS);
            current_seq++;
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval));
            int total_pack = 1 + (payload_len - 1) / pack_size;
            for (int i = 0; i < total_pack; i++)
            {
                int to_send = min<int>(pack_size, payload_len - i * pack_size);
                auto frame = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) + sizeof(int) + to_send]);
                memcpy(frame.get(), &frame_counter, sizeof(frame_counter));
                memcpy(frame.get() + sizeof(frame_counter), &current_seq, sizeof(current_seq));
                memcpy(frame.get() + sizeof(frame_counter) + sizeof(current_seq), encoded.data() + i * pack_size, to_send);
                stream->push_frame(frame.get(), sizeof(uint32_t) + sizeof(int) + to_send, RTP_NO_FLAGS);
                current_seq++;
                std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval));
            }
            frame_counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval));
        }
        sess->destroy_stream(stream);
    }
    if (sess)
        ctx.destroy_session(sess);
}