// videoworker.cpp - Version 1.1 (Fixed signal/slot mismatch)
#include "videoworker.h"
#include <QDebug>
#include <QThread>

VideoWorker::VideoWorker(QObject *parent) : QObject(parent)
{
    m_processor = new VideoProcessor();
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this, &VideoWorker::onPlaybackTimerTimeout);
}

VideoWorker::~VideoWorker()
{
    m_playbackTimer->stop();
    delete m_processor;
}

void VideoWorker::processOpenFile(const QString &filePath)
{
    bool success = m_processor->openFile(filePath);
    if (success) {
        VideoProcessor::AudioParams params = m_processor->getAudioParams();
        double frameRate = m_processor->getFrameRate();
        qint64 duration = m_processor->getDuration();
        // SỬA LỖI: Lấy timeBase và gửi đi cùng signal
        AVRational timeBase = m_processor->getTimeBase();
        emit fileOpened(true, params, frameRate, duration, timeBase);

        // Tự động phát frame đầu tiên
        FrameData firstFrame = m_processor->seekAndDecode(0);
        if(!firstFrame.image.isNull()) {
            m_currentPts = firstFrame.pts;
            emit frameReady(firstFrame);
        }
    } else {
        // SỬA LỖI: Gửi đi một timeBase rỗng
        emit fileOpened(false, {}, 0.0, 0, {0, 1});
    }
}

void VideoWorker::processSeek(qint64 timestamp)
{
    FrameData frame = m_processor->seekAndDecode(timestamp);
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    }
}

void VideoWorker::processPlayPause(bool play)
{
    m_isPlaying = play;
    if (m_isPlaying) {
        double frameRate = m_processor->getFrameRate();
        if (frameRate > 0) {
            m_playbackTimer->start(1000 / frameRate);
        }
    } else {
        m_playbackTimer->stop();
    }
}

void VideoWorker::processNextFrame()
{
    if (m_isPlaying) return; // Chỉ cho phép khi đang pause
    onPlaybackTimerTimeout();
}

void VideoWorker::processPrevFrame()
{
    if (m_isPlaying) return; // Chỉ cho phép khi đang pause

    AVRational timeBase = m_processor->getTimeBase();
    if (timeBase.den == 0) return;
    double frameRate = m_processor->getFrameRate();
    if(frameRate == 0.0) return;

    qint64 currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
    double frameDurationUs = 1000000.0 / frameRate;
    qint64 prevTimeUs = qMax((qint64)0, currentTimeUs - (long long)(1.5 * frameDurationUs));
    
    FrameData frame = m_processor->seekAndDecode(prevTimeUs);
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    }
}

void VideoWorker::stop()
{
    m_isPlaying = false;
    m_playbackTimer->stop();
    if (m_processor) {
        m_processor->stop_processing = true;
    }
    emit finished();
}

void VideoWorker::onPlaybackTimerTimeout()
{
    FrameData frame = m_processor->decodeNextFrame();
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    } else {
        // Video kết thúc
        m_isPlaying = false;
        m_playbackTimer->stop();
    }
}
