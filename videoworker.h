// videoworker.h - Version 1.1 (Fixed signal/slot mismatch)
#ifndef VIDEOWORKER_H
#define VIDEOWORKER_H

#include <QObject>
#include <QTimer>
#include "videoprocessor.h"

class VideoWorker : public QObject
{
    Q_OBJECT

public:
    explicit VideoWorker(QObject *parent = nullptr);
    ~VideoWorker();

public slots:
    void processOpenFile(const QString &filePath);
    void processSeek(qint64 timestamp);
    void processPlayPause(bool play);
    void processNextFrame();
    void processPrevFrame();
    void stop();

signals:
    // SỬA LỖI: Thêm AVRational timeBase để khớp với slot
    void fileOpened(bool success, VideoProcessor::AudioParams params, double frameRate, qint64 duration, AVRational timeBase);
    void frameReady(const FrameData &frameData);
    void finished();

private slots:
    void onPlaybackTimerTimeout();

private:
    VideoProcessor *m_processor;
    QTimer *m_playbackTimer;
    bool m_isPlaying = false;
    qint64 m_currentPts = 0;
};

#endif // VIDEOWORKER_H
