// videoprocessor.cpp - Version 1.3
#include "videoprocessor.h"
#include <QDebug>

VideoProcessor::VideoProcessor() {}

VideoProcessor::~VideoProcessor()
{
    cleanup();
}

bool VideoProcessor::openFile(const QString &filePath)
{
    cleanup();
    std::string filePathStr = filePath.toStdString();
    if (avformat_open_input(&formatContext, filePathStr.c_str(), nullptr, nullptr) != 0) return false;
    if (avformat_find_stream_info(formatContext, nullptr) < 0) { cleanup(); return false; }

    videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (videoStreamIndex < 0) { cleanup(); return false; }

    const AVCodecParameters *codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) { cleanup(); return false; }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) { cleanup(); return false; }
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) { cleanup(); return false; }
    if (avcodec_open2(codecContext, codec, nullptr) < 0) { cleanup(); return false; }

    qDebug() << "Successfully opened video file:" << filePath;
    return true;
}

FrameData VideoProcessor::decodeNextFrame()
{
    if (!formatContext) return {};

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    FrameData result;

    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecContext, &packet) == 0) {
                int response = avcodec_receive_frame(codecContext, frame);
                if (response == 0) {
                    result.image = convertFrameToImage(frame);
                    result.pts = frame->pts;
                    av_packet_unref(&packet);
                    av_frame_free(&frame);
                    return result;
                }
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    return {};
}

FrameData VideoProcessor::seekAndDecode(int64_t target_ts_us)
{
    if (!seek(target_ts_us)) {
        return {};
    }

    // Sau khi seek đến keyframe, giải mã liên tục cho đến khi đạt được frame mong muốn
    FrameData frameData;
    while (true) {
        frameData = decodeNextFrame();
        if (frameData.image.isNull()) {
            // Hết video, trả về frame rỗng
            return frameData;
        }

        AVRational timeBase = getTimeBase();
        if (timeBase.den == 0) return {}; // Tránh lỗi chia cho 0

        int64_t current_ts_us = frameData.pts * 1000000 * timeBase.num / timeBase.den;

        if (current_ts_us >= target_ts_us) {
            // Đã tìm thấy frame tại hoặc ngay sau thời điểm mong muốn
            return frameData;
        }
    }
}

bool VideoProcessor::seek(int64_t timestamp)
{
    if (!formatContext) return false;
    int64_t seek_target = av_rescale(timestamp, formatContext->streams[videoStreamIndex]->time_base.den, formatContext->streams[videoStreamIndex]->time_base.num) / 1000000;
    if (av_seek_frame(formatContext, videoStreamIndex, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        return false;
    }
    avcodec_flush_buffers(codecContext);
    return true;
}

int64_t VideoProcessor::getDuration() const
{
    if (formatContext) {
        return formatContext->duration;
    }
    return 0;
}

AVRational VideoProcessor::getTimeBase() const
{
    if (formatContext && videoStreamIndex >= 0) {
        return formatContext->streams[videoStreamIndex]->time_base;
    }
    return {0, 1};
}

double VideoProcessor::getFrameRate() const
{
    if (formatContext && videoStreamIndex >= 0) {
        AVRational fr = formatContext->streams[videoStreamIndex]->avg_frame_rate;
        return (double)fr.num / fr.den;
    }
    return 0.0;
}

QImage VideoProcessor::convertFrameToImage(AVFrame* frame)
{
    if (!frame) return QImage();
    swsContext = sws_getCachedContext(swsContext,
                                      frame->width, frame->height, (AVPixelFormat)frame->format,
                                      frame->width, frame->height, AV_PIX_FMT_RGB24,
                                      SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsContext) return QImage();
    QImage image(frame->width, frame->height, QImage::Format_RGB888);
    uint8_t* const data[] = { image.bits() };
    const int linesize[] = { static_cast<int>(image.bytesPerLine()) };
    sws_scale(swsContext, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, data, linesize);
    return image;
}

void VideoProcessor::cleanup()
{
    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
    }
    videoStreamIndex = -1;
    codec = nullptr;
}
