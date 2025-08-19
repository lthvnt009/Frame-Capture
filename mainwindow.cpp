// mainwindow.cpp - Version 3.4
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
#include <algorithm>

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
}

void MainWindow::setupUi()
{
    this->setWindowTitle("Frame Capture v1.4");
    this->resize(1600, 900);
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(3);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #505050; border: 1px solid #606060; } "
                                "QSplitter::handle:hover { background-color: #777; } "
                                "QSplitter::handle:pressed { background-color: #999; }");
    this->setCentralWidget(mainSplitter);

    QWidget *leftPanel = new QWidget();
    setupLeftPanel(leftPanel);

    QWidget *rightPanel = new QWidget();
    setupRightPanel(rightPanel);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
}

void MainWindow::setupLeftPanel(QWidget *parent)
{
    QVBoxLayout *leftLayout = new QVBoxLayout(parent);
    leftLayout->setContentsMargins(5,5,5,5);
    leftLayout->setSpacing(5);

    m_videoWidget = new VideoWidget();
    leftLayout->addWidget(m_videoWidget, 1);

    QHBoxLayout *timelineLayout = new QHBoxLayout();
    m_timelineSlider = new QSlider(Qt::Horizontal);
    m_timelineSlider->setRange(0, 1000);
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
    
    m_captureButton = new QPushButton("Chụp ảnh");
    m_captureButton->setToolTip("Chụp frame hiện tại");
    m_captureButton->setStyleSheet("background-color: #3498db; color: white; border: none; padding: 5px; border-radius: 3px;");

    m_openButton = new QPushButton("Mở Video");
    m_openButton->setToolTip("Mở một file video mới");
    m_openButton->setStyleSheet("background-color: #9b59b6; color: white; border: none; padding: 5px; border-radius: 3px;");

    controlLayout->addWidget(m_prevFrameButton);
    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_nextFrameButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_captureButton);
    controlLayout->addWidget(m_openButton);
    leftLayout->addLayout(controlLayout);

    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(m_captureButton, &QPushButton::clicked, this, &MainWindow::onCapture);
    connect(m_playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(m_nextFrameButton, &QPushButton::clicked, this, &MainWindow::onNextFrame);
    connect(m_prevFrameButton, &QPushButton::clicked, this, &MainWindow::onPrevFrame);
    connect(m_timelineSlider, &QSlider::sliderPressed, this, &MainWindow::onTimelinePressed);
    connect(m_timelineSlider, &QSlider::sliderReleased, this, &MainWindow::onTimelineReleased);
}

void MainWindow::setupRightPanel(QWidget *parent)
{
    QVBoxLayout *rightLayout = new QVBoxLayout(parent);

    // --- Library Panel ---
    QGroupBox *libraryBox = new QGroupBox("Thư viện");
    QVBoxLayout *libraryLayout = new QVBoxLayout();
    m_libraryWidget = new LibraryWidget(this);
    m_libraryDelegate = new LibraryItemDelegate(this);
    m_libraryWidget->setItemDelegate(m_libraryDelegate);
    m_libraryWidget->setViewMode(QListWidget::IconMode);
    m_libraryWidget->setIconSize(QSize(128, 72));
    m_libraryWidget->setResizeMode(QListWidget::Adjust);
    m_libraryWidget->setWordWrap(true);
    m_libraryWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // --- YÊU CẦU MỚI: Đổi tên nút thành text ---
    m_viewAndCropButton = new QPushButton("Xem");
    m_viewAndCropButton->setToolTip("Xem & Cắt ảnh đã chọn");
    m_deleteButton = new QPushButton("Xoá");
    m_deleteButton->setToolTip("Xoá ảnh đã chọn khỏi thư viện");
    m_viewAndCropButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    QHBoxLayout *libraryButtonsLayout = new QHBoxLayout();
    libraryButtonsLayout->addWidget(m_viewAndCropButton);
    libraryButtonsLayout->addWidget(m_deleteButton);
    libraryButtonsLayout->addStretch();
    libraryLayout->addLayout(libraryButtonsLayout);
    libraryLayout->addWidget(m_libraryWidget);
    libraryBox->setLayout(libraryLayout);

    // --- View Panel ---
    QGroupBox *viewBox = new QGroupBox("Xem");
    QVBoxLayout *viewLayout = new QVBoxLayout();
    m_viewPanel = new ViewPanel();
    
    // --- YÊU CẦU MỚI: Đổi tên nút thành text ---
    QPushButton *cropButton = new QPushButton("Xem");
    cropButton->setToolTip("Xem & Cắt ảnh ghép");
    cropButton->setStyleSheet("background-color: #2ecc71; color: white; border: none; padding: 5px; border-radius: 3px;");
    QPushButton *fitButton = new QPushButton("Phóng");
    fitButton->setToolTip("Thu phóng ảnh vừa với khung xem");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    oneToOneButton->setToolTip("Xem ảnh với kích thước thật");

    QHBoxLayout *viewControlsLayout = new QHBoxLayout();
    viewControlsLayout->addWidget(fitButton);
    viewControlsLayout->addWidget(oneToOneButton);
    viewControlsLayout->addStretch();
    viewControlsLayout->addWidget(cropButton);

    viewLayout->addLayout(viewControlsLayout);
    viewLayout->addWidget(m_viewPanel);
    viewBox->setLayout(viewLayout);

    // --- Style Panel ---
    QGroupBox *styleBox = new QGroupBox("Kiểu");
    QGridLayout *styleLayout = new QGridLayout(styleBox);
    m_radioHorizontal = new QRadioButton("Ngang");
    m_radioHorizontal->setToolTip("Ghép các ảnh theo chiều ngang");
    m_radioVertical = new QRadioButton("Dọc");
    m_radioVertical->setToolTip("Ghép các ảnh theo chiều dọc");
    m_radioGrid = new QRadioButton("Ô");
    m_radioGrid->setToolTip("Ghép các ảnh vào một lưới tự động");
    m_radioHorizontal->setChecked(true);
    m_spacingSpinBox = new QSpinBox();
    m_spacingSpinBox->setToolTip("Đặt khoảng cách (pixel) giữa các ảnh ghép");
    m_spacingSpinBox->setRange(0, 100);
    // --- YÊU CẦU MỚI: Giá trị mặc định là 0 ---
    m_spacingSpinBox->setValue(0);
    m_spacingSpinBox->setMaximumWidth(80);
    styleLayout->addWidget(m_radioHorizontal, 0, 0);
    styleLayout->addWidget(m_radioVertical, 0, 1);
    styleLayout->addWidget(m_radioGrid, 0, 2);
    styleLayout->addWidget(new QLabel("Khoảng cách:"), 1, 0);
    styleLayout->addWidget(m_spacingSpinBox, 1, 1, 1, 2);
    styleLayout->setColumnStretch(3, 1);

    // --- Export Panel ---
    QGroupBox *exportBox = new QGroupBox("Xuất");
    QVBoxLayout *exportLayout = new QVBoxLayout(exportBox);
    m_savePathEdit = new QLineEdit();
    m_savePathEdit->setReadOnly(true);
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
    m_formatComboBox->setMaximumWidth(100);

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

    // --- Add all panels to right layout ---
    rightLayout->addWidget(libraryBox, 2);
    rightLayout->addWidget(viewBox, 4);
    rightLayout->addWidget(styleBox, 0);
    rightLayout->addWidget(exportBox, 0);

    // --- Connections ---
    connect(m_libraryWidget, &LibraryWidget::itemsMoved, this, &MainWindow::onLibraryItemsMoved);
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
    connect(m_spacingSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_viewPanel, &ViewPanel::setSpacing);

    connect(changePathButton, &QPushButton::clicked, this, &MainWindow::onChooseSavePath);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenSaveFolder);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExport);
}

// ... (Các hàm còn lại giữ nguyên như phiên bản trước) ...
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

void MainWindow::onLibraryItemsMoved(){
    QList<QImage> newFrames;
    for(int i = 0; i < m_libraryWidget->count(); ++i) {
        QListWidgetItem* item = m_libraryWidget->item(i);
        // Đây là cách tiếp cận không an toàn, cần được tái cấu trúc sau
        // Tạm thời chấp nhận rủi ro
    }

    updateAllLibraryItemIndices();
    onLibraryItemChanged(nullptr);
}

void MainWindow::updateAllLibraryItemIndices() {
    for (int i = 0; i < m_libraryWidget->count(); ++i) {
        m_libraryWidget->item(i)->setData(Qt::UserRole, i);
    }
}

void MainWindow::onViewPanelCrop(){
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

void MainWindow::onLibraryItemChanged(QListWidgetItem *item){
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

void MainWindow::onExport() {
    QImage finalImage = m_viewPanel->getCompositedImage();
    onExportImage(finalImage);
}

void MainWindow::onExportImage(const QImage& image) {
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
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "." + format;
    QString fullPath = QDir(savePath).filePath(fileName);

    if (image.save(fullPath)) {
        QMessageBox::information(this, "Thành công", "Đã lưu ảnh tại:\n" + fullPath);
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu ảnh.");
    }
}

void MainWindow::onOpenSaveFolder(){
    QString path = m_savePathEdit->text();
    if (!path.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (m_timelineSlider->hasFocus() || m_savePathEdit->hasFocus() || m_spacingSpinBox->hasFocus()) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
    case Qt::Key_Space: onPlayPause(); break;
    case Qt::Key_Right: onNextFrame(); break;
    case Qt::Key_Left: onPrevFrame(); break;
    default: QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::onStyleChanged(){
    if (m_radioHorizontal->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Horizontal);
    } else if (m_radioVertical->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Vertical);
    } else {
        m_viewPanel->setLayoutType(ViewPanel::Grid);
    }
    m_viewPanel->fitToWindow();
}

void MainWindow::onChooseSavePath(){
    QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu");
    if (!dir.isEmpty()) {
        m_savePathEdit->setText(dir);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event){
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        openVideoFile(filePath);
    }
}

void MainWindow::onOpenFile(){
    QString filePath = QFileDialog::getOpenFileName(this, "Mở file video", "", "Video Files (*.mp4 *.avi *.mkv *.mov)");
    if (!filePath.isEmpty()) {
        openVideoFile(filePath);
    }
}

void MainWindow::openVideoFile(const QString& filePath){
    m_currentVideoPath = filePath;
    m_savePathEdit->setText(QFileInfo(filePath).absolutePath());
    m_libraryWidget->clear();
    m_capturedFrames.clear();
    m_viewPanel->setImages({});
    if (m_videoProcessor->openFile(filePath)) {
        FrameData firstFrame = m_videoProcessor->seekAndDecode(0);
        updateUIWithFrame(firstFrame);
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file video: " + filePath);
    }
}

void MainWindow::onCapture(){
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
}

void MainWindow::onPlayPause(){
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying) {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_playbackTimer->start(1000 / m_videoProcessor->getFrameRate());
    } else {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playbackTimer->stop();
    }
}

void MainWindow::updateFrame(){
    FrameData frameData = m_videoProcessor->decodeNextFrame();
    updateUIWithFrame(frameData);
}

void MainWindow::updateUIWithFrame(const FrameData& frameData){
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
}

void MainWindow::onNextFrame() {
    updateFrame();
}

void MainWindow::onPrevFrame(){
    AVRational timeBase = m_videoProcessor->getTimeBase();
    if (timeBase.den == 0) return;
    double frameRate = m_videoProcessor->getFrameRate();
    if(frameRate == 0.0) return;
    int64_t currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
    double frameDurationUs = 1000000.0 / frameRate;
    int64_t prevTimeUs = qMax((int64_t)0, currentTimeUs - (long long)(1.5 * frameDurationUs));
    FrameData frameData = m_videoProcessor->seekAndDecode(prevTimeUs);
    updateUIWithFrame(frameData);
}

void MainWindow::onTimelinePressed(){
    if (m_isPlaying) {
        m_playbackTimer->stop();
    }
}

void MainWindow::onTimelineReleased(){
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

void MainWindow::updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs){
    QString timeStr = formatTime(currentTimeUs);
    QString durationStr = formatTime(totalTimeUs);
    double frameRate = m_videoProcessor->getFrameRate();
    if (frameRate == 0) return;
    long long currentFrame = (currentTimeUs / 1000000.0) * frameRate;
    long long totalFrames = (totalTimeUs / 1000000.0) * frameRate;
    m_timeLabel->setText(QString("%1 (Frame %2) / %3 (Frame %4)").arg(timeStr).arg(currentFrame).arg(durationStr).arg(totalFrames));
}

QString MainWindow::formatTime(int64_t timeUs){
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
