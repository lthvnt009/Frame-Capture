// mainwindow.h - Version 3.2
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QImage>
#include <QFrame>
#include <QMouseEvent>
#include "videoprocessor.h"
#include "viewpanel.h"

// --- Helper class cho ô màu có thể click ---
// Lớp này phải được định nghĩa trong file header để AUTOMOC hoạt động chính xác
class ClickableFrame : public QFrame {
    Q_OBJECT
public:
    explicit ClickableFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QFrame::mousePressEvent(event);
    }
};


// --- Forward declarations cho các lớp khác ---
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
class QAction;


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
    void onCaptureAndExport();
    void onMuteClicked();
    void onVolumeChanged(int volume);

    // Library Panel
    void onLibrarySelectionChanged();
    void onViewAndCropSelected();
    void onDeleteSelected();
    void onLibraryItemQuickExport(QListWidgetItem *item);

    // View Panel
    void onLibraryItemChanged(QListWidgetItem *item);
    void onViewPanelCrop();
    void onStyleChanged();
    void updateViewPanelScaleLabel(double scale);

    // Export Panel
    void onExport();
    void onExportImage(const QImage& image);
    void onChooseSavePath();
    void onOpenSaveFolder();

    // Style Panel Slots
    void onChooseBackgroundColor();
    void onCornerRadiusSliderChanged(int value);
    void onBorderSliderChanged(int value);

private:
    void setupUi();
    void setupLeftPanel(QWidget *parent);
    void setupRightPanel(QWidget *parent);
    QWidget* createVerticalSpinBox(QSpinBox* spinbox);
    void openVideoFile(const QString& filePath);
    void updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs);
    void updateUIWithFrame(const FrameData& frameData);
    QString formatTime(int64_t timeUs);
    void updateAllLibraryItemIndices();
    void cleanupAudio();
    QString generateUniqueFilename(const QString& baseName, const QString& extension);

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
    QPushButton *m_captureAndExportButton;
    QAction *m_captureExportAction;
    QSlider *m_timelineSlider;
    QLabel *m_timeLabel;
    QPushButton *m_muteButton;
    QSlider *m_volumeSlider;
    
    // Audio
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    float m_lastVolume = 1.0f;

    // Right Panel Components
    LibraryWidget *m_libraryWidget;
    LibraryItemDelegate *m_libraryDelegate;
    QPushButton *m_viewAndCropButton;
    QPushButton *m_deleteButton;

    ViewPanel *m_viewPanel;
    QLineEdit *m_viewScaleLabel;
    QRadioButton *m_radioGrid, *m_radioVertical, *m_radioHorizontal;
    QSpinBox *m_spacingSpinBox;
    
    // Style controls
    QSpinBox *m_borderSpinBox;
    QSlider *m_borderSlider;
    QSpinBox *m_cornerRadiusSpinBox;
    QSlider *m_cornerRadiusSlider;
    ClickableFrame *m_colorSwatch;

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
