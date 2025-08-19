// mainwindow.h - Version 2.9
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QImage>
#include "videoprocessor.h"
#include "viewpanel.h"

// Forward declarations
class QSplitter;
class VideoWidget;
class QPushButton;
class QSlider;
class QLabel;
class QTimer;
class QDragEnterEvent;
class QDropEvent;
class QKeyEvent;
class LibraryWidget;
class QListWidgetItem;
class QGroupBox;
class QRadioButton;
class LibraryItemDelegate;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QAudioSink;
class QIODevice;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Media Player
    void onOpenFile();
    void onPlayPause();
    void onNextFrame();
    void onPrevFrame();
    void onTimelinePressed();
    void onTimelineReleased();
    void updateFrame();
    void onCapture();
    void onMuteClicked();
    void onVolumeChanged(int volume);

    // Library Panel
    void onLibrarySelectionChanged();
    void onViewAndCropSelected();
    void onDeleteSelected();

    // View Panel
    void onLibraryItemChanged(QListWidgetItem *item);
    void onViewPanelCrop();
    void onStyleChanged();

    // Export Panel
    void onExport();
    void onExportImage(const QImage& image);
    void onChooseSavePath();
    void onOpenSaveFolder();

private:
    void setupUi();
    void setupLeftPanel(QWidget *parent);
    void setupRightPanel(QWidget *parent);
    void openVideoFile(const QString& filePath);
    void updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs);
    void updateUIWithFrame(const FrameData& frameData);
    QString formatTime(int64_t timeUs);
    void updateAllLibraryItemIndices();
    void cleanupAudio();

    // Layout
    QSplitter *mainSplitter;

    // Left Panel Components
    VideoProcessor *m_videoProcessor;
    VideoWidget *m_videoWidget;
    QPushButton *m_openButton;
    QPushButton *m_playPauseButton;
    QPushButton *m_nextFrameButton;
    QPushButton *m_prevFrameButton;
    QPushButton *m_captureButton;
    QSlider *m_timelineSlider;
    QLabel *m_timeLabel;
    QPushButton *m_muteButton;
    QSlider *m_volumeSlider;
    
    // Audio
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    float m_lastVolume = 1.0f; // SỬA LỖI: Lưu âm lượng trước khi tắt tiếng

    // Right Panel Components
    LibraryWidget *m_libraryWidget;
    LibraryItemDelegate *m_libraryDelegate;
    QPushButton *m_viewAndCropButton;
    QPushButton *m_deleteButton;

    ViewPanel *m_viewPanel;
    QRadioButton *m_radioGrid, *m_radioVertical, *m_radioHorizontal;
    QSpinBox *m_spacingSpinBox;

    QComboBox *m_formatComboBox;
    QLineEdit *m_savePathEdit;
    QPushButton *m_exportButton;

    // Data
    QTimer *m_playbackTimer;
    bool m_isPlaying = false;
    int64_t m_currentPts = 0;
    QList<QImage> m_capturedFrames;
    QString m_currentVideoPath;
};
#endif // MAINWINDOW_H
