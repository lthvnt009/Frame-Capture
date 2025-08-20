// mainwindow.cpp - Version 4.4
#include "mainwindow.h"
#include "videowidget.h"
#include "viewpanel.h"
#include "librarywidget.h"
#include "libraryitemdelegate.h"
#include "cropdialog.h"
#include "imageviewerdialog.h"

#include <QSplitter>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QStyle>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>
#include <QListWidget>
#include <QIcon>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>
#include <QMap>
#include <algorithm>
#include <QAudioSink>
#include <QMediaDevices>
#include <QColorDialog>
#include <QFrame>
#include <QAction>
#include <QToolButton>

// Lớp ClickableFrame đã được chuyển sang mainwindow.h

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_videoProcessor = new VideoProcessor();
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this, &MainWindow::updateFrame);
    setupUi();
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    delete m_videoProcessor;
    cleanupAudio();
}

void MainWindow::setupUi()
{
    this->setWindowTitle("Frame Capture v2.4");
    this->resize(1600, 900);
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(1);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #606060; }");
    this->setCentralWidget(mainSplitter);

    QWidget *leftPanel = new QWidget();
    setupLeftPanel(leftPanel);

    QWidget *rightPanel = new QWidget();
    setupRightPanel(rightPanel);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({1000, 1000}); 
}

void MainWindow::setupLeftPanel(QWidget *parent)
{
    QGroupBox *playerBox = new QGroupBox("Trình Phát", parent);
    QVBoxLayout *leftLayout = new QVBoxLayout(playerBox);
    leftLayout->setContentsMargins(5,5,5,5);
    leftLayout->setSpacing(5);

    m_videoWidget = new VideoWidget();
    leftLayout->addWidget(m_videoWidget, 1);

    QHBoxLayout *timelineLayout = new QHBoxLayout();
    m_timelineSlider = new QSlider(Qt::Horizontal);
    m_timelineSlider->setRange(0, 1000);
    m_timelineSlider->setFocusPolicy(Qt::NoFocus);
    m_timeLabel = new QLabel("00:00.000 / 00:00.000");
    timelineLayout->addWidget(m_timelineSlider);
    timelineLayout->addWidget(m_timeLabel);
    leftLayout->addLayout(timelineLayout);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_prevFrameButton = new QPushButton();
    m_prevFrameButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    m_prevFrameButton->setToolTip("Frame trước (Phím ←)");
    m_playPauseButton = new QPushButton();
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playPauseButton->setToolTip("Phát/Dừng (Phím Space)");
    m_nextFrameButton = new QPushButton();
    m_nextFrameButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    m_nextFrameButton->setToolTip("Frame kế tiếp (Phím →)");
    
    QSize buttonIconSize(36, 36);
    m_prevFrameButton->setIconSize(buttonIconSize);
    m_playPauseButton->setIconSize(buttonIconSize);
    m_nextFrameButton->setIconSize(buttonIconSize);

    m_muteButton = new QPushButton();
    m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_muteButton->setToolTip("Tắt/Mở tiếng");
    m_muteButton->setCheckable(true);
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setMaximumWidth(80);

    m_captureAndExportButton = new QPushButton("Chụp và Xuất");
    m_captureAndExportButton->setToolTip("Chụp frame hiện tại và xuất trực tiếp ra file (Ctrl+Space)");
    m_captureAndExportButton->setStyleSheet("background-color: #27ae60; color: white; border: none; padding: 5px; border-radius: 3px;");
    
    m_captureExportAction = new QAction(this);
    m_captureExportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space));
    connect(m_captureExportAction, &QAction::triggered, this, &MainWindow::onCaptureAndExport);
    this->addAction(m_captureExportAction);

    m_captureButton = new QPushButton("Chụp");
    m_captureButton->setToolTip("Chụp frame hiện tại vào thư viện");
    m_captureButton->setStyleSheet("background-color: #3498db; color: white; border: none; padding: 5px; border-radius: 3px;");

    m_openButton = new QPushButton("Mở Video");
    m_openButton->setToolTip("Mở một file video mới");
    m_openButton->setStyleSheet("background-color: #9b59b6; color: white; border: none; padding: 5px; border-radius: 3px;");

    controlLayout->addWidget(m_prevFrameButton);
    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_nextFrameButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_muteButton);
    controlLayout->addWidget(m_volumeSlider);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_captureAndExportButton);
    controlLayout->addWidget(m_captureButton);
    controlLayout->addWidget(m_openButton);
    leftLayout->addLayout(controlLayout);
    
    QVBoxLayout* parentLayout = new QVBoxLayout(parent);
    parentLayout->addWidget(playerBox);

    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(m_captureButton, &QPushButton::clicked, this, &MainWindow::onCapture);
    connect(m_captureAndExportButton, &QPushButton::clicked, this, &MainWindow::onCaptureAndExport);
    connect(m_playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(m_nextFrameButton, &QPushButton::clicked, this, &MainWindow::onNextFrame);
    connect(m_prevFrameButton, &QPushButton::clicked, this, &MainWindow::onPrevFrame);
    connect(m_timelineSlider, &QSlider::sliderPressed, this, &MainWindow::onTimelinePressed);
    connect(m_timelineSlider, &QSlider::sliderReleased, this, &MainWindow::onTimelineReleased);
    connect(m_muteButton, &QPushButton::clicked, this, &MainWindow::onMuteClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);
}

QWidget* MainWindow::createVerticalSpinBox(QSpinBox* spinbox) {
    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);

    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    QToolButton *upButton = new QToolButton();
    upButton->setArrowType(Qt::UpArrow);
    upButton->setFixedSize(12, 11);
    QToolButton *downButton = new QToolButton();
    downButton->setArrowType(Qt::DownArrow);
    downButton->setFixedSize(12, 11);
    buttonLayout->addWidget(upButton);
    buttonLayout->addWidget(downButton);

    layout->addWidget(spinbox);
    layout->addLayout(buttonLayout);

    connect(upButton, &QToolButton::clicked, spinbox, &QSpinBox::stepUp);
    connect(downButton, &QToolButton::clicked, spinbox, &QSpinBox::stepDown);
    return container;
}

void MainWindow::setupRightPanel(QWidget *parent)
{
    QVBoxLayout *rightLayout = new QVBoxLayout(parent);

    QGroupBox *libraryBox = new QGroupBox("Thư viện");
    QVBoxLayout *libraryLayout = new QVBoxLayout();
    m_libraryWidget = new LibraryWidget(this);
    m_libraryDelegate = new LibraryItemDelegate(this);
    m_libraryWidget->setItemDelegate(m_libraryDelegate);
    m_libraryWidget->setViewMode(QListWidget::IconMode);
    m_libraryWidget->setIconSize(QSize(128, 72));
    m_libraryWidget->setWordWrap(true);
    m_libraryWidget->setSpacing(2); 
    connect(m_libraryWidget, &LibraryWidget::itemQuickExportRequested, this, &MainWindow::onLibraryItemQuickExport);

    m_viewAndCropButton = new QPushButton("Xem");
    m_viewAndCropButton->setToolTip("Xem & Cắt ảnh đã chọn trong thư viện");
    m_viewAndCropButton->setStyleSheet("background-color: #2980b9; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_deleteButton = new QPushButton("Xoá");
    m_deleteButton->setToolTip("Xoá ảnh đã chọn khỏi thư viện");
    m_deleteButton->setStyleSheet("background-color: #c0392b; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_viewAndCropButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    QHBoxLayout *libraryButtonsLayout = new QHBoxLayout();
    libraryButtonsLayout->addStretch();
    libraryButtonsLayout->addWidget(m_viewAndCropButton);
    libraryButtonsLayout->addWidget(m_deleteButton);
    libraryLayout->addLayout(libraryButtonsLayout);
    libraryLayout->addWidget(m_libraryWidget);
    libraryBox->setLayout(libraryLayout);

    QGroupBox *viewBox = new QGroupBox("Xem");
    QVBoxLayout *viewLayout = new QVBoxLayout();
    m_viewPanel = new ViewPanel();
    connect(m_viewPanel, &ViewPanel::scaleChanged, this, &MainWindow::updateViewPanelScaleLabel);
    
    QPushButton *cropButton = new QPushButton("Xem");
    cropButton->setToolTip("Mở cửa sổ xem và cắt ảnh ghép");
    cropButton->setStyleSheet("background-color: #2980b9; color: white; border: none; padding: 5px; border-radius: 3px;");
    
    QPushButton *fitButton = new QPushButton("Phóng");
    fitButton->setToolTip("Thu phóng ảnh vừa với khung xem");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    oneToOneButton->setToolTip("Xem ảnh với kích thước thật");

    m_viewScaleLabel = new QLineEdit("100%");
    m_viewScaleLabel->setReadOnly(true);
    m_viewScaleLabel->setAlignment(Qt::AlignCenter);
    m_viewScaleLabel->setFixedSize(50, 22);
    m_viewScaleLabel->setToolTip("Tỉ lệ phóng hiện tại");
    m_viewScaleLabel->setStyleSheet("background-color: #2c3e50; color: white; border: 1px solid #606060;");

    QHBoxLayout *viewControlsLayout = new QHBoxLayout();
    viewControlsLayout->addWidget(m_viewScaleLabel);
    viewControlsLayout->addWidget(fitButton);
    viewControlsLayout->addWidget(oneToOneButton);
    viewControlsLayout->addStretch();
    viewControlsLayout->addWidget(cropButton);

    viewLayout->addLayout(viewControlsLayout);
    viewLayout->addWidget(m_viewPanel);
    viewBox->setLayout(viewLayout);

    QGroupBox *styleBox = new QGroupBox("Kiểu");
    // YÊU CẦU: Tái cấu trúc panel Kiểu
    QHBoxLayout *mainStyleLayout = new QHBoxLayout(styleBox);

    // -- Cột Trái
    QVBoxLayout *leftStyleLayout = new QVBoxLayout();
    QGroupBox *layoutTypeBox = new QGroupBox("Bố cục");
    QHBoxLayout *radioLayout = new QHBoxLayout(layoutTypeBox);
    m_radioHorizontal = new QRadioButton("Ngang");
    m_radioHorizontal->setToolTip("Ghép các ảnh theo chiều ngang");
    m_radioVertical = new QRadioButton("Dọc");
    m_radioVertical->setToolTip("Ghép các ảnh theo chiều dọc");
    m_radioGrid = new QRadioButton("Ô");
    m_radioGrid->setToolTip("Ghép các ảnh vào một lưới tự động");
    m_radioHorizontal->setChecked(true);
    radioLayout->addWidget(m_radioHorizontal);
    radioLayout->addWidget(m_radioVertical);
    radioLayout->addWidget(m_radioGrid);
    leftStyleLayout->addWidget(layoutTypeBox);

    QGridLayout *leftControlsLayout = new QGridLayout();
    leftControlsLayout->setColumnStretch(2, 1);
    m_spacingSpinBox = new QSpinBox();
    m_spacingSpinBox->setToolTip("Đặt khoảng cách (pixel) giữa các ảnh ghép");
    m_spacingSpinBox->setRange(0, 100);
    m_spacingSpinBox->setValue(0);
    m_spacingSpinBox->setFixedWidth(40);
    QSlider* spacingSlider = new QSlider(Qt::Horizontal);
    spacingSlider->setRange(0, 100);
    leftControlsLayout->addWidget(new QLabel("Khoảng cách:"), 0, 0);
    leftControlsLayout->addWidget(createVerticalSpinBox(m_spacingSpinBox), 0, 1);
    leftControlsLayout->addWidget(spacingSlider, 0, 2);

    m_colorSwatch = new ClickableFrame();
    m_colorSwatch->setToolTip("Nhấn để chọn màu nền");
    m_colorSwatch->setFrameShape(QFrame::Box);
    m_colorSwatch->setFrameShadow(QFrame::Sunken);
    m_colorSwatch->setAutoFillBackground(true);
    m_colorSwatch->setFixedSize(24, 24);
    QPalette pal = m_colorSwatch->palette();
    pal.setColor(QPalette::Window, m_viewPanel->palette().color(QPalette::Window));
    m_colorSwatch->setPalette(pal);
    connect(m_colorSwatch, &ClickableFrame::clicked, this, &MainWindow::onChooseBackgroundColor);
    leftControlsLayout->addWidget(new QLabel("Nền:"), 1, 0);
    leftControlsLayout->addWidget(m_colorSwatch, 1, 1);
    leftStyleLayout->addLayout(leftControlsLayout);
    leftStyleLayout->addStretch();

    // -- Cột Phải
    QVBoxLayout *rightStyleLayout = new QVBoxLayout();
    QGridLayout *rightControlsLayout = new QGridLayout();
    rightControlsLayout->setColumnStretch(2, 1);
    m_borderSpinBox = new QSpinBox();
    m_borderSpinBox->setToolTip("Đặt độ rộng viền (pixel)");
    m_borderSpinBox->setRange(0, 200);
    m_borderSpinBox->setValue(0);
    m_borderSpinBox->setFixedWidth(40);
    m_borderSlider = new QSlider(Qt::Horizontal);
    m_borderSlider->setRange(0, 200);
    rightControlsLayout->addWidget(new QLabel("Viền:"), 0, 0);
    rightControlsLayout->addWidget(createVerticalSpinBox(m_borderSpinBox), 0, 1);
    rightControlsLayout->addWidget(m_borderSlider, 0, 2);

    m_cornerRadiusSpinBox = new QSpinBox();
    m_cornerRadiusSpinBox->setToolTip("Đặt độ bo tròn góc (%)");
    m_cornerRadiusSpinBox->setRange(0, 50);
    m_cornerRadiusSpinBox->setValue(0);
    m_cornerRadiusSpinBox->setSuffix("%");
    m_cornerRadiusSpinBox->setFixedWidth(40);
    m_cornerRadiusSlider = new QSlider(Qt::Horizontal);
    m_cornerRadiusSlider->setRange(0, 50);
    rightControlsLayout->addWidget(new QLabel("Bo Viền:"), 1, 0);
    rightControlsLayout->addWidget(createVerticalSpinBox(m_cornerRadiusSpinBox), 1, 1);
    rightControlsLayout->addWidget(m_cornerRadiusSlider, 1, 2);
    rightStyleLayout->addLayout(rightControlsLayout);
    rightStyleLayout->addStretch();

    mainStyleLayout->addLayout(leftStyleLayout);
    mainStyleLayout->addSpacing(20);
    mainStyleLayout->addLayout(rightStyleLayout);

    QGroupBox *exportBox = new QGroupBox("Xuất");
    QVBoxLayout *exportLayout = new QVBoxLayout(exportBox);
    m_savePathEdit = new QLineEdit();
    m_savePathEdit->setReadOnly(true);
    m_savePathEdit->setToolTip("Thư mục sẽ lưu ảnh xuất ra");
    QPushButton *changePathButton = new QPushButton("Thay đổi");
    changePathButton->setToolTip("Chọn thư mục để lưu ảnh");
    QPushButton *openFolderButton = new QPushButton();
    openFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    openFolderButton->setToolTip("Mở thư mục lưu hiện tại");
    m_exportButton = new QPushButton("Xuất ảnh");
    m_exportButton->setToolTip("Lưu ảnh ghép trong panel 'Xem' thành file");
    m_exportButton->setStyleSheet("background-color: #e67e22; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_formatComboBox = new QComboBox();
    m_formatComboBox->setToolTip("Chọn định dạng file ảnh để lưu");
    m_formatComboBox->addItems({"PNG", "JPG", "BMP", "TIFF", "WEBP"});

    QHBoxLayout *saveLineLayout = new QHBoxLayout();
    saveLineLayout->addWidget(new QLabel("Nơi lưu:"));
    saveLineLayout->addWidget(m_savePathEdit, 1);
    saveLineLayout->addWidget(changePathButton);
    saveLineLayout->addWidget(openFolderButton);

    QHBoxLayout *formatLineLayout = new QHBoxLayout();
    formatLineLayout->addWidget(new QLabel("Định dạng:"));
    formatLineLayout->addWidget(m_formatComboBox);
    formatLineLayout->addStretch();
    formatLineLayout->addWidget(m_exportButton);

    exportLayout->addLayout(saveLineLayout);
    exportLayout->addLayout(formatLineLayout);

    rightLayout->addWidget(libraryBox, 2);
    rightLayout->addWidget(viewBox, 4);
    rightLayout->addWidget(styleBox, 0);
    rightLayout->addWidget(exportBox, 0);

    connect(m_libraryWidget, &QListWidget::itemChanged, this, &MainWindow::onLibraryItemChanged);
    connect(m_libraryWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onLibrarySelectionChanged);
    connect(m_viewAndCropButton, &QPushButton::clicked, this, &MainWindow::onViewAndCropSelected);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
    connect(cropButton, &QPushButton::clicked, this, &MainWindow::onViewPanelCrop);
    connect(fitButton, &QPushButton::clicked, m_viewPanel, &ViewPanel::fitToWindow);
    connect(oneToOneButton, &QPushButton::clicked, m_viewPanel, &ViewPanel::setOneToOne);
    connect(m_radioHorizontal, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    connect(m_radioVertical, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    connect(m_radioGrid, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    
    connect(m_spacingSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), spacingSlider, &QSlider::setValue);
    connect(spacingSlider, &QSlider::valueChanged, m_spacingSpinBox, &QSpinBox::setValue);
    connect(spacingSlider, &QSlider::valueChanged, m_viewPanel, &ViewPanel::setSpacing);
    
    connect(m_borderSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_borderSlider, &QSlider::setValue);
    connect(m_borderSlider, &QSlider::valueChanged, m_borderSpinBox, &QSpinBox::setValue);
    connect(m_borderSlider, &QSlider::valueChanged, this, &MainWindow::onBorderSliderChanged);
    
    connect(m_cornerRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_cornerRadiusSlider, &QSlider::setValue);
    connect(m_cornerRadiusSlider, &QSlider::valueChanged, m_cornerRadiusSpinBox, &QSpinBox::setValue);
    connect(m_cornerRadiusSlider, &QSlider::valueChanged, this, &MainWindow::onCornerRadiusSliderChanged);

    connect(changePathButton, &QPushButton::clicked, this, &MainWindow::onChooseSavePath);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenSaveFolder);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExport);
}

void MainWindow::updateViewPanelScaleLabel(double scale)
{
    m_viewScaleLabel->setText(QString::number(qRound(scale * 100)) + "%");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        openVideoFile(filePath);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (qobject_cast<QLineEdit*>(focusWidget()) || qobject_cast<QSpinBox*>(focusWidget())) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
    case Qt::Key_Space: 
        onPlayPause(); 
        event->accept();
        break;
    case Qt::Key_Right: 
        onNextFrame(); 
        event->accept();
        break;
    case Qt::Key_Left: 
        onPrevFrame(); 
        event->accept();
        break;
    default: 
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::onOpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Mở file video", "", "Video Files (*.mp4 *.avi *.mkv *.mov)");
    if (!filePath.isEmpty()) {
        openVideoFile(filePath);
    }
}

void MainWindow::onPlayPause()
{
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying) {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_playbackTimer->start(1000 / m_videoProcessor->getFrameRate());
    } else {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playbackTimer->stop();
    }
    this->setFocus();
}

void MainWindow::onNextFrame() 
{
    updateFrame();
    this->setFocus();
}

void MainWindow::onPrevFrame()
{
    AVRational timeBase = m_videoProcessor->getTimeBase();
    if (timeBase.den == 0) return;
    double frameRate = m_videoProcessor->getFrameRate();
    if(frameRate == 0.0) return;
    int64_t currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
    double frameDurationUs = 1000000.0 / frameRate;
    int64_t prevTimeUs = qMax((int64_t)0, currentTimeUs - (long long)(1.5 * frameDurationUs));
    FrameData frameData = m_videoProcessor->seekAndDecode(prevTimeUs);
    updateUIWithFrame(frameData);
    this->setFocus();
}

void MainWindow::onTimelinePressed()
{
    if (m_isPlaying) {
        m_playbackTimer->stop();
    }
}

void MainWindow::onTimelineReleased()
{
    int position = m_timelineSlider->value();
    int64_t duration = m_videoProcessor->getDuration();
    if (duration > 0) {
        int64_t targetTime = duration * (double)position / 1000.0;
        FrameData frameData = m_videoProcessor->seekAndDecode(targetTime);
        updateUIWithFrame(frameData);
    }
    if (m_isPlaying) {
        m_playbackTimer->start(1000 / m_videoProcessor->getFrameRate());
    }
    this->setFocus();
}

void MainWindow::updateFrame()
{
    FrameData frameData = m_videoProcessor->decodeNextFrame();
    updateUIWithFrame(frameData);
}

void MainWindow::onCapture()
{
    QImage currentFrame = m_videoWidget->getCurrentImage();
    if (!currentFrame.isNull()) {
        m_capturedFrames.append(currentFrame);
        int newIndex = m_capturedFrames.count() - 1;
        QPixmap thumbnail = QPixmap::fromImage(currentFrame.scaled(m_libraryWidget->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QListWidgetItem *item = new QListWidgetItem(QIcon(thumbnail), "");
        item->setData(Qt::UserRole, newIndex);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_libraryWidget->addItem(item);
    }
    this->setFocus();
}

void MainWindow::onCaptureAndExport()
{
    QImage currentFrame = m_videoWidget->getCurrentImage();
    if (!currentFrame.isNull()) {
        onExportImage(currentFrame);
    }
    this->setFocus();
}

void MainWindow::onMuteClicked()
{
    if (m_volumeSlider->value() > 0) {
        m_lastVolume = m_volumeSlider->value() / 100.0f;
        m_volumeSlider->setValue(0);
    } else {
        m_volumeSlider->setValue(m_lastVolume * 100);
    }
}

void MainWindow::onVolumeChanged(int volume)
{
    if (m_audioSink) {
        m_audioSink->setVolume(volume / 100.0f);
    }
    
    bool isMutedNow = (volume == 0);
    m_muteButton->setChecked(isMutedNow);
    m_muteButton->setIcon(style()->standardIcon(isMutedNow ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));

    if (!isMutedNow) {
        m_lastVolume = volume / 100.0f;
    }
}

void MainWindow::onLibrarySelectionChanged()
{
    bool hasSelection = !m_libraryWidget->selectedItems().isEmpty();
    m_viewAndCropButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onViewAndCropSelected()
{
    QList<QListWidgetItem*> selected = m_libraryWidget->selectedItems();
    if (selected.isEmpty()) return;

    QListWidgetItem* item = selected.first();
    int index = item->data(Qt::UserRole).toInt();

    if (index >= 0 && index < m_capturedFrames.size()) {
        CropDialog dialog(m_capturedFrames[index], this);
        connect(&dialog, &CropDialog::exportImageRequested, this, &MainWindow::onExportImage);
        
        if (dialog.exec() == QDialog::Accepted) {
            QImage finalImage = dialog.getFinalImage();
            if (!finalImage.isNull() && finalImage != m_capturedFrames[index]) {
                m_capturedFrames[index] = finalImage;
                QPixmap thumbnail = QPixmap::fromImage(finalImage.scaled(m_libraryWidget->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                item->setIcon(QIcon(thumbnail));
                onLibraryItemChanged(item);
            }
        }
    }
}

void MainWindow::onDeleteSelected()
{
    QList<QListWidgetItem*> selected = m_libraryWidget->selectedItems();
    if (selected.isEmpty()) return;

    int ret = QMessageBox::question(this, "Xác nhận xoá", "Bạn có chắc muốn xoá ảnh đã chọn?", QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        int rowToDelete = m_libraryWidget->row(selected.first());
        QListWidgetItem* item = m_libraryWidget->takeItem(rowToDelete);
        int originalIndex = item->data(Qt::UserRole).toInt();

        if (originalIndex >= 0 && originalIndex < m_capturedFrames.size()) {
            m_capturedFrames.removeAt(originalIndex);
        }
        delete item;

        updateAllLibraryItemIndices();
        onLibraryItemChanged(nullptr);
    }
}

void MainWindow::onLibraryItemQuickExport(QListWidgetItem *item)
{
    if (!item) return;
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_capturedFrames.size()) {
        onExportImage(m_capturedFrames[index]);
    }
}

void MainWindow::onLibraryItemChanged(QListWidgetItem *item)
{
    Q_UNUSED(item);
    QList<QImage> checkedImages;
    for (int i = 0; i < m_libraryWidget->count(); ++i) {
        QListWidgetItem* currentItem = m_libraryWidget->item(i);
        if (currentItem->checkState() == Qt::Checked) {
            int index = currentItem->data(Qt::UserRole).toInt();
            if (index >= 0 && index < m_capturedFrames.size()) {
                checkedImages.append(m_capturedFrames[index]);
            }
        }
    }
    m_viewPanel->setImages(checkedImages);
    m_viewPanel->fitToWindow();
}

void MainWindow::onViewPanelCrop()
{
    QImage imageToCrop = m_viewPanel->getCompositedImage();
    if (imageToCrop.isNull()) {
        QMessageBox::information(this, "Thông báo", "Không có ảnh nào trong vùng xem để cắt.");
        return;
    }
    CropDialog dialog(imageToCrop, this);
    connect(&dialog, &CropDialog::exportImageRequested, this, &MainWindow::onExportImage);

    if (dialog.exec() == QDialog::Accepted) {
        QImage finalImage = dialog.getFinalImage();
        if(!finalImage.isNull()) {
            m_viewPanel->setImages({finalImage});
            m_viewPanel->fitToWindow();
        }
    }
}

void MainWindow::onStyleChanged()
{
    if (m_radioHorizontal->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Horizontal);
    } else if (m_radioVertical->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Vertical);
    } else {
        m_viewPanel->setLayoutType(ViewPanel::Grid);
    }
    m_viewPanel->fitToWindow();
}

void MainWindow::onExport() 
{
    QImage finalImage = m_viewPanel->getCompositedImage();
    onExportImage(finalImage);
}

void MainWindow::onExportImage(const QImage& image) 
{
    if (image.isNull()) {
        QMessageBox::warning(this, "Lỗi", "Không có ảnh để xuất.");
        return;
    }
    QString savePath = m_savePathEdit->text();
    if (savePath.isEmpty()) {
        savePath = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu");
        if(savePath.isEmpty()) return;
        m_savePathEdit->setText(savePath);
    }
    
    QString format = m_formatComboBox->currentText().toLower();
    QString baseName = QFileInfo(m_currentVideoPath).baseName();
    if (baseName.isEmpty()) {
        baseName = "capture";
    }
    QString fullPath = generateUniqueFilename(baseName, format);

    if (image.save(fullPath)) {
        QMessageBox::information(this, "Thành công", "Đã lưu ảnh tại:\n" + fullPath);
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu ảnh.");
    }
}

void MainWindow::onChooseSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu");
    if (!dir.isEmpty()) {
        m_savePathEdit->setText(dir);
    }
}

void MainWindow::onOpenSaveFolder()
{
    QString path = m_savePathEdit->text();
    if (!path.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs)
{
    QString timeStr = formatTime(currentTimeUs);
    QString durationStr = formatTime(totalTimeUs);
    double frameRate = m_videoProcessor->getFrameRate();
    if (frameRate == 0) return;
    long long currentFrame = (currentTimeUs / 1000000.0) * frameRate;
    long long totalFrames = (totalTimeUs / 1000000.0) * frameRate;
    m_timeLabel->setText(QString("%1 (Frame %2) / %3 (Frame %4)").arg(timeStr).arg(currentFrame).arg(durationStr).arg(totalFrames));
}

QString MainWindow::formatTime(int64_t timeUs)
{
    int totalMilliseconds = timeUs / 1000;
    int seconds = (totalMilliseconds / 1000) % 60;
    int minutes = (totalMilliseconds / (1000 * 60)) % 60;
    int hours = (totalMilliseconds / (1000 * 60 * 60));
    int milliseconds = totalMilliseconds % 1000;
    if (hours > 0)
        return QString("%1:%2:%3.%4").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
    else
        return QString("%1:%2.%3").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
}

void MainWindow::updateAllLibraryItemIndices() 
{
    for (int i = 0; i < m_libraryWidget->count(); ++i) {
        m_libraryWidget->item(i)->setData(Qt::UserRole, i);
    }
}

void MainWindow::cleanupAudio()
{
    if(m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_audioDevice = nullptr;
    }
}

QString MainWindow::generateUniqueFilename(const QString& baseName, const QString& extension)
{
    QString savePath = m_savePathEdit->text();
    QString fullPath = QDir(savePath).filePath(baseName + "." + extension);
    int counter = 1;
    while (QFile::exists(fullPath)) {
        fullPath = QDir(savePath).filePath(QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
        counter++;
    }
    return fullPath;
}

void MainWindow::openVideoFile(const QString& filePath)
{
    cleanupAudio();
    m_currentVideoPath = filePath;
    m_savePathEdit->setText(QFileInfo(filePath).absolutePath());
    m_libraryWidget->clear();
    m_capturedFrames.clear();
    m_viewPanel->setImages({});
    if (m_videoProcessor->openFile(filePath)) {
        VideoProcessor::AudioParams params = m_videoProcessor->getAudioParams();
        if(params.isValid) {
            QAudioFormat format;
            format.setSampleRate(params.sample_rate);
            format.setChannelCount(params.channels);
            format.setSampleFormat(QAudioFormat::Int16);
            
            const auto& devices = QMediaDevices::defaultAudioOutput();
            if(!devices.isDefault()) {
                 QMessageBox::warning(this, "Lỗi Âm thanh", "Không tìm thấy thiết bị âm thanh mặc định.");
            } else {
                m_audioSink = new QAudioSink(devices, format);
                m_audioDevice = m_audioSink->start();
                onVolumeChanged(m_volumeSlider->value());
            }
        }

        FrameData firstFrame = m_videoProcessor->seekAndDecode(0);
        updateUIWithFrame(firstFrame);
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file video: " + filePath);
    }
}

void MainWindow::updateUIWithFrame(const FrameData& frameData)
{
    if (!frameData.image.isNull()) {
        m_videoWidget->setImage(frameData.image);
        m_currentPts = frameData.pts;
        AVRational timeBase = m_videoProcessor->getTimeBase();
        if (timeBase.den == 0) return;
        int64_t currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
        int64_t totalTimeUs = m_videoProcessor->getDuration();
        updateTimeLabel(currentTimeUs, totalTimeUs);
        m_timelineSlider->blockSignals(true);
        if (totalTimeUs > 0) {
            m_timelineSlider->setValue((double)currentTimeUs / totalTimeUs * 1000);
        }
        m_timelineSlider->blockSignals(false);
    } else {
        if (m_isPlaying) {
            onPlayPause();
        }
    }
    if(m_audioDevice && !frameData.audioData.isEmpty()) {
        m_audioDevice->write(frameData.audioData);
    }
}

void MainWindow::onCornerRadiusSliderChanged(int value)
{
    m_viewPanel->setCornerRadius(value);
}

void MainWindow::onBorderSliderChanged(int value)
{
    m_viewPanel->setBorder(value);
}

void MainWindow::onChooseBackgroundColor()
{
    QColor color = QColorDialog::getColor(m_viewPanel->palette().window().color(), this, "Chọn màu nền");
    if (color.isValid()) {
        m_viewPanel->setBackgroundColor(color);
        QPalette pal = m_colorSwatch->palette();
        pal.setColor(QPalette::Window, color);
        m_colorSwatch->setPalette(pal);
    }
}
