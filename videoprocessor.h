// videoprocessor.h - Version 1.3
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QString>
#include <QImage>

// FFmpeg headers must be wrapped in extern "C" for C++ compatibility
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

struct FrameData {
    QImage image;
    int64_t pts = 0; // Presentation timestamp
};

class VideoProcessor
{
public:
    VideoProcessor();
    ~VideoProcessor();

    bool openFile(const QString &filePath);
    FrameData decodeNextFrame();
    FrameData seekAndDecode(int64_t timestamp); // Hàm mới

    // Getters
    int64_t getDuration() const;
    AVRational getTimeBase() const;
    double getFrameRate() const; // Hàm mới

private:
    void cleanup();
    QImage convertFrameToImage(AVFrame* frame);
    bool seek(int64_t timestamp); // Chuyển thành private

    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    const AVCodec *codec = nullptr;
    SwsContext *swsContext = nullptr;

    int videoStreamIndex = -1;
};

#endif // VIDEOPROCESSOR_H
