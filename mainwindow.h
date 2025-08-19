// mainwindow.h - Version 2.4
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

    // Panels
    void onLibraryItemChanged(QListWidgetItem *item);
    void onLibraryItemsMoved();
    void onStyleChanged();
    void onExport();
    void onChooseSavePath();
    void onOpenSaveFolder();
    void onViewDetail();
    void onFitView();
    void onOneToOne();
    void onViewSelected();
    void onDeleteSelected();
    void updateLibrarySelection();

private:
    void setupUi();
    void setupLeftPanel(QWidget *parent);
    void setupRightPanel(QWidget *parent);
    void openVideoFile(const QString& filePath);
    void updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs);
    void updateUIWithFrame(const FrameData& frameData);
    QString formatTime(int64_t timeUs);

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

    // Right Panel Components
    LibraryWidget *m_libraryWidget;
    LibraryItemDelegate *m_libraryDelegate;
    ViewPanel *m_viewPanel;
    QRadioButton *m_radioGrid, *m_radioVertical, *m_radioHorizontal;
    QSpinBox *m_spacingSpinBox;
    QComboBox *m_formatComboBox;
    QLineEdit *m_savePathEdit;
    QPushButton *m_exportButton;
    QPushButton *m_viewSelectedButton;
    QPushButton *m_deleteSelectedButton;

    // Data
    QTimer *m_playbackTimer;
    bool m_isPlaying = false;
    int64_t m_currentPts = 0;
    QList<QImage> m_capturedFrames;
    QString m_currentVideoPath;
};
#endif // MAINWINDOW_H
