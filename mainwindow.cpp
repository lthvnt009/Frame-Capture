// mainwindow.cpp - Version 2.8 (Hoàn chỉnh)
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
    this->setWindowTitle("Frame Capture v1.0");
    this->resize(1600, 900);
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(5);
    mainSplitter->setStyleSheet("QSplitter::handle { background: #555; } QSplitter::handle:hover { background: #777; }");
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
    m_playPauseButton = new QPushButton();
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_nextFrameButton = new QPushButton();
    m_nextFrameButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    m_captureButton = new QPushButton("Chụp ảnh");
    m_captureButton->setStyleSheet("background-color: #3498db; color: white; border: none; padding: 5px; border-radius: 3px;");

    m_openButton = new QPushButton("Mở Video");

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

    QGroupBox *libraryBox = new QGroupBox("Thư viện");
    QVBoxLayout *libraryLayout = new QVBoxLayout();
    m_libraryWidget = new LibraryWidget(this);
    m_libraryDelegate = new LibraryItemDelegate(this);
    m_libraryWidget->setItemDelegate(m_libraryDelegate);
    m_libraryWidget->setViewMode(QListWidget::IconMode);
    m_libraryWidget->setIconSize(QSize(128, 72));
    m_libraryWidget->setResizeMode(QListWidget::Adjust);
    m_libraryWidget->setWordWrap(true);
    m_viewSelectedButton = new QPushButton("Xem");
    m_deleteSelectedButton = new QPushButton("Xoá");
    m_viewSelectedButton->setEnabled(false);
    m_deleteSelectedButton->setEnabled(false);
    QHBoxLayout *libraryButtonsLayout = new QHBoxLayout();
    libraryButtonsLayout->addWidget(m_viewSelectedButton);
    libraryButtonsLayout->addWidget(m_deleteSelectedButton);
    libraryButtonsLayout->addStretch();
    libraryLayout->addLayout(libraryButtonsLayout);
    libraryLayout->addWidget(m_libraryWidget);
    libraryBox->setLayout(libraryLayout);

    QGroupBox *viewBox = new QGroupBox("Xem");
    QVBoxLayout *viewLayout = new QVBoxLayout();
    m_viewPanel = new ViewPanel();
    QPushButton *cropButton = new QPushButton("Cắt ảnh");
    cropButton->setStyleSheet("background-color: #2ecc71; color: white; border: none; padding: 5px; border-radius: 3px;");
    QPushButton *fitButton = new QPushButton("Phóng");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    QHBoxLayout *viewControlsLayout = new QHBoxLayout();
    viewControlsLayout->addWidget(cropButton);
    viewControlsLayout->addStretch();
    viewControlsLayout->addWidget(fitButton);
    viewControlsLayout->addWidget(oneToOneButton);
    viewLayout->addLayout(viewControlsLayout);
    viewLayout->addWidget(m_viewPanel);
    viewBox->setLayout(viewLayout);

    QGroupBox *styleBox = new QGroupBox("Kiểu");
    QGridLayout *styleLayout = new QGridLayout(styleBox);
    m_radioHorizontal = new QRadioButton("Ngang");
    m_radioVertical = new QRadioButton("Dọc");
    m_radioGrid = new QRadioButton("Ô");
    m_radioHorizontal->setChecked(true);
    m_spacingSpinBox = new QSpinBox();
    m_spacingSpinBox->setRange(0, 100);
    m_spacingSpinBox->setValue(5);
    m_spacingSpinBox->setMaximumWidth(100);
    styleLayout->addWidget(m_radioHorizontal, 0, 0);
    styleLayout->addWidget(m_radioVertical, 0, 1);
    styleLayout->addWidget(m_radioGrid, 0, 2);
    styleLayout->addWidget(new QLabel("Khoảng cách:"), 1, 0);
    styleLayout->addWidget(m_spacingSpinBox, 1, 1, 1, 2);

    QGroupBox *exportBox = new QGroupBox("Xuất");
    QVBoxLayout *exportLayout = new QVBoxLayout(exportBox);
    m_formatComboBox = new QComboBox();
    m_formatComboBox->addItems({"PNG", "JPG", "BMP", "TIFF", "WEBP"});
    m_savePathEdit = new QLineEdit();
    m_savePathEdit->setReadOnly(true);
    QPushButton *changePathButton = new QPushButton("Thay đổi");
    QPushButton *openFolderButton = new QPushButton();
    openFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_exportButton = new QPushButton("Xuất ảnh");
    m_exportButton->setStyleSheet("background-color: #e67e22; color: white; border: none; padding: 5px; border-radius: 3px;");
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(m_savePathEdit);
    pathLayout->addWidget(changePathButton);
    pathLayout->addWidget(openFolderButton);
    exportLayout->addWidget(new QLabel("Định dạng:"));
    exportLayout->addWidget(m_formatComboBox);
    exportLayout->addWidget(new QLabel("Nơi lưu:"));
    exportLayout->addLayout(pathLayout);
    QHBoxLayout *exportButtonLayout = new QHBoxLayout();
    exportButtonLayout->addWidget(m_exportButton);
    exportButtonLayout->addStretch();
    exportLayout->addLayout(exportButtonLayout);

    rightLayout->addWidget(libraryBox, 2);
    rightLayout->addWidget(viewBox, 3);
    rightLayout->addWidget(styleBox);
    rightLayout->addWidget(exportBox);

    connect(m_libraryWidget, &LibraryWidget::itemsMoved, this, &MainWindow::onLibraryItemsMoved);
    connect(m_libraryWidget, &QListWidget::itemChanged, this, &MainWindow::onLibraryItemChanged);
    connect(m_libraryWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::updateLibrarySelection);
    connect(m_viewSelectedButton, &QPushButton::clicked, this, &MainWindow::onViewSelected);
    connect(m_deleteSelectedButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
    connect(cropButton, &QPushButton::clicked, this, &MainWindow::onViewDetail);
    connect(m_radioHorizontal, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    connect(m_radioVertical, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    connect(m_radioGrid, &QRadioButton::toggled, this, &MainWindow::onStyleChanged);
    connect(m_spacingSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_viewPanel, &ViewPanel::setSpacing);
    connect(changePathButton, &QPushButton::clicked, this, &MainWindow::onChooseSavePath);
    connect(openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenSaveFolder);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(fitButton, &QPushButton::clicked, this, &MainWindow::onFitView);
    connect(oneToOneButton, &QPushButton::clicked, this, &MainWindow::onOneToOne);
    connect(m_viewPanel, &ViewPanel::scaleChanged, this, [this](int val){ m_viewPanel->setScale(val / 100.0); });
}

void MainWindow::onFitView()
{
    m_viewPanel->setScale(1.0);
}

void MainWindow::onOneToOne()
{
    m_viewPanel->setScale(1.0);
}

void MainWindow::onLibraryItemsMoved(){
    QList<QImage> reorderedFrames;
    for (int i = 0; i < m_libraryWidget->count(); ++i) {
        QListWidgetItem *item = m_libraryWidget->item(i);
        int originalIndex = item->data(Qt::UserRole).toInt();
        if (originalIndex >= 0 && originalIndex < m_capturedFrames.size()) {
            reorderedFrames.append(m_capturedFrames[originalIndex]);
        }
    }
    m_capturedFrames = reorderedFrames;

    for (int i = 0; i < m_libraryWidget->count(); ++i) {
        m_libraryWidget->item(i)->setData(Qt::UserRole, i);
    }
    onLibraryItemChanged(nullptr);
}

void MainWindow::onOpenSaveFolder(){
    QString path = m_savePathEdit->text();
    if (!path.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::onViewDetail(){
    QImage imageToCrop = m_viewPanel->getCompositedImage();
    if (imageToCrop.isNull()) {
        QMessageBox::information(this, "Thông báo", "Không có ảnh nào trong vùng xem để cắt.");
        return;
    }
    CropDialog dialog(imageToCrop, this);
    if (dialog.exec() == QDialog::Accepted) {
        QImage croppedImage = dialog.getCroppedImage();
        m_viewPanel->setImages({croppedImage});
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (m_timelineSlider->hasFocus()) {
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
}

void MainWindow::onStyleChanged(){
    if (m_radioHorizontal->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Horizontal);
    } else if (m_radioVertical->isChecked()) {
        m_viewPanel->setLayoutType(ViewPanel::Vertical);
    } else {
        m_viewPanel->setLayoutType(ViewPanel::Grid);
    }
}

void MainWindow::onChooseSavePath(){
    QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu");
    if (!dir.isEmpty()) {
        m_savePathEdit->setText(dir);
    }
}

void MainWindow::onExport(){
    QString savePath = m_savePathEdit->text();
    if (savePath.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng chọn thư mục lưu.");
        return;
    }
    QImage finalImage = m_viewPanel->getCompositedImage();
    if (finalImage.isNull()) {
        QMessageBox::warning(this, "Lỗi", "Không có ảnh để xuất.");
        return;
    }
    QString format = m_formatComboBox->currentText().toLower();
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "." + format;
    QString fullPath = savePath + "/" + fileName;
    if (finalImage.save(fullPath)) {
        QMessageBox::information(this, "Thành công", "Đã lưu ảnh tại:\n" + fullPath);
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu ảnh.");
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
    int64_t currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
    double frameDurationUs = 1000000.0 / m_videoProcessor->getFrameRate();
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

void MainWindow::updateLibrarySelection()
{
    bool hasSelection = !m_libraryWidget->selectedItems().isEmpty();
    m_viewSelectedButton->setEnabled(hasSelection);
    m_deleteSelectedButton->setEnabled(hasSelection);
}

void MainWindow::onViewSelected()
{
    QList<QListWidgetItem*> selected = m_libraryWidget->selectedItems();
    if (selected.isEmpty()) return;

    int index = selected.first()->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_capturedFrames.size()) {
        ImageViewerDialog dialog(m_capturedFrames[index], this);
        dialog.exec();
    }
}

void MainWindow::onDeleteSelected()
{
    QList<QListWidgetItem*> selected = m_libraryWidget->selectedItems();
    if (selected.isEmpty()) return;

    int ret = QMessageBox::question(this, "Xác nhận xoá",
                                    QString("Bạn có chắc muốn xoá %1 ảnh đã chọn?").arg(selected.count()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QList<int> rowsToDelete;
        for(auto item : selected) {
            rowsToDelete.append(m_libraryWidget->row(item));
        }
        std::sort(rowsToDelete.rbegin(), rowsToDelete.rend());

        for(int row : rowsToDelete) {
            QListWidgetItem* item = m_libraryWidget->takeItem(row);
            int originalIndex = item->data(Qt::UserRole).toInt();
            if (originalIndex >= 0 && originalIndex < m_capturedFrames.size()) {
                m_capturedFrames[originalIndex] = QImage();
            }
            delete item;
        }

        m_capturedFrames.removeAll(QImage());

        for (int i = 0; i < m_libraryWidget->count(); ++i) {
            m_libraryWidget->item(i)->setData(Qt::UserRole, i);
        }

        onLibraryItemChanged(nullptr);
    }
}
