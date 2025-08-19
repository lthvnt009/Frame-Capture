// cropdialog.cpp - Version 2.4
#include "cropdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QApplication>
#include <QScrollArea>
#include <QButtonGroup>
#include <QShowEvent>
#include <QMessageBox>

// --- Triển khai CropArea ---
CropArea::CropArea(const QImage &image, QWidget *parent)
    : QWidget(parent), m_image(image)
{
    setMouseTracking(true);
    m_selectionRect = QRectF();
    setFixedSize(m_image.size());
}

QRect CropArea::getSelection() const
{
    return m_selectionRect.isValid() ? m_selectionRect.normalized().toRect() : QRect();
}

void CropArea::setAspectRatio(double ratio)
{
    m_aspectRatio = ratio;
    if (m_aspectRatio > 0 && m_selectionRect.isValid()) {
        double currentHeight = m_selectionRect.width() / m_aspectRatio;
        m_selectionRect.setHeight(currentHeight);
    }
    update();
}

void CropArea::setScale(double newScale)
{
    m_scale = newScale;
    setFixedSize(m_image.size() * m_scale);
    update();
}

void CropArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.scale(m_scale, m_scale);
    painter.drawImage(0, 0, m_image);

    if (m_selectionRect.isValid()) {
        QPainterPath path;
        path.addRect(m_image.rect());
        path.addRect(m_selectionRect);
        painter.setBrush(QColor(0, 0, 0, 128));
        painter.drawPath(path);

        painter.setPen(QPen(Qt::white, 1 / m_scale, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selectionRect);

        painter.setPen(QPen(Qt::white, 1 / m_scale));
        painter.setBrush(Qt::white);
        for (int i = TopLeft; i <= Left; ++i) {
            painter.drawRect(getHandleRect(static_cast<Handle>(i)));
        }
    }
}

void CropArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->pos() / m_scale;
        m_currentHandle = getHandleAt(pos);
        m_dragStartPos = pos;
        if (m_currentHandle == None) {
            m_selectionRect.setTopLeft(pos);
            m_selectionRect.setSize({0,0});
            m_currentHandle = BottomRight;
        }
    }
}

void CropArea::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = event->pos() / m_scale;
    if (m_currentHandle != None) {
        resizeSelection(pos);
    } else {
        updateCursor(pos);
    }
}

void CropArea::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_currentHandle = None;
}

void CropArea::wheelEvent(QWheelEvent *event)
{
    double newScale = m_scale + (event->angleDelta().y() > 0 ? 0.1 : -0.1);
    newScale = qBound(0.1, newScale, 5.0);
    setScale(newScale);
}

void CropArea::updateCursor(const QPointF &pos)
{
    Handle handle = getHandleAt(pos);
    switch (handle) {
    case TopLeft: case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case TopRight: case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;
    case Move: setCursor(Qt::SizeAllCursor); break;
    default: setCursor(Qt::CrossCursor); break;
    }
}

void CropArea::resizeSelection(const QPointF &pos)
{
    QRectF originalRect = m_selectionRect;
    QPointF delta = pos - m_dragStartPos;

    switch (m_currentHandle) {
        case TopLeft: m_selectionRect.setTopLeft(originalRect.topLeft() + delta); break;
        case Top: m_selectionRect.setTop(originalRect.top() + delta.y()); break;
        case TopRight: m_selectionRect.setTopRight(originalRect.topRight() + delta); break;
        case Right: m_selectionRect.setRight(originalRect.right() + delta.x()); break;
        case BottomRight: m_selectionRect.setBottomRight(originalRect.bottomRight() + delta); break;
        case Bottom: m_selectionRect.setBottom(originalRect.bottom() + delta.y()); break;
        case BottomLeft: m_selectionRect.setBottomLeft(originalRect.bottomLeft() + delta); break;
        case Left: m_selectionRect.setLeft(originalRect.left() + delta.x()); break;
        case Move: m_selectionRect.translate(delta); break;
        default: break;
    }

    if (m_aspectRatio > 0) {
        double w = m_selectionRect.width();
        double h = m_selectionRect.height();
        if (m_currentHandle == Top || m_currentHandle == Bottom || m_currentHandle == TopLeft || m_currentHandle == TopRight || m_currentHandle == BottomLeft || m_currentHandle == BottomRight) {
             h = w / m_aspectRatio;
             m_selectionRect.setHeight(h);
        } else {
             w = h * m_aspectRatio;
             m_selectionRect.setWidth(w);
        }
    }

    m_dragStartPos = pos;
    update();
}

CropArea::Handle CropArea::getHandleAt(const QPointF &pos) const
{
    if (!m_selectionRect.isValid()) return None;
    for (int i = TopLeft; i <= Left; ++i) {
        if (getHandleRect(static_cast<Handle>(i)).contains(pos)) {
            return static_cast<Handle>(i);
        }
    }
    if (m_selectionRect.contains(pos)) {
        return Move;
    }
    return None;
}

QRectF CropArea::getHandleRect(Handle handle) const
{
    double size = 8 / m_scale;
    QPointF center;
    switch (handle) {
        case TopLeft: center = m_selectionRect.topLeft(); break;
        case Top: center = {m_selectionRect.center().x(), m_selectionRect.top()}; break;
        case TopRight: center = m_selectionRect.topRight(); break;
        case Right: center = {m_selectionRect.right(), m_selectionRect.center().y()}; break;
        case BottomRight: center = m_selectionRect.bottomRight(); break;
        case Bottom: center = {m_selectionRect.center().x(), m_selectionRect.bottom()}; break;
        case BottomLeft: center = m_selectionRect.bottomLeft(); break;
        case Left: center = {m_selectionRect.left(), m_selectionRect.center().y()}; break;
        default: return QRectF();
    }
    return QRectF(center.x() - size/2, center.y() - size/2, size, size);
}

// --- Triển khai CropDialog ---
CropDialog::CropDialog(const QImage &image, QWidget *parent)
    : QDialog(parent), m_sourceImage(image)
{
    setupUi();
    setWindowTitle("Cắt ảnh");
    resize(800, 600);
}

QImage CropDialog::getCroppedImage() const
{
    return m_croppedImage;
}

void CropDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    fitToWindow();
}

void CropDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        acceptCrop();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void CropDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_cropArea = new CropArea(m_sourceImage, this);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidget(m_cropArea);
    mainLayout->addWidget(m_scrollArea, 1);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QGroupBox *ratioBox = new QGroupBox("Tỉ lệ");
    QHBoxLayout *ratioLayout = new QHBoxLayout();
    m_ratioGroup = new QButtonGroup(this);
    m_ratioGroup->setExclusive(false);
    QRadioButton *freeformRadio = new QRadioButton("Tự do");
    QRadioButton *ratio11 = new QRadioButton("1:1");
    QRadioButton *ratio43 = new QRadioButton("4:3");
    QRadioButton *ratio169 = new QRadioButton("16:9");
    m_ratioGroup->addButton(freeformRadio, 0);
    m_ratioGroup->addButton(ratio11, 1);
    m_ratioGroup->addButton(ratio43, 2);
    m_ratioGroup->addButton(ratio169, 3);
    ratioLayout->addWidget(freeformRadio);
    ratioLayout->addWidget(ratio11);
    ratioLayout->addWidget(ratio43);
    ratioLayout->addWidget(ratio169);
    ratioBox->setLayout(ratioLayout);

    QPushButton *fitButton = new QPushButton("Phóng");
    QPushButton *oneToOneButton = new QPushButton("1:1");

    controlsLayout->addWidget(ratioBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(fitButton);
    controlsLayout->addWidget(oneToOneButton);
    mainLayout->addLayout(controlsLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton("Xuất", QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_ratioGroup, &QButtonGroup::idToggled, this, &CropDialog::onAspectRatioChanged);
    connect(fitButton, &QPushButton::clicked, this, &CropDialog::fitToWindow);
    connect(oneToOneButton, &QPushButton::clicked, this, &CropDialog::oneToOne);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CropDialog::acceptCrop);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void CropDialog::onAspectRatioChanged(int id, bool checked)
{
    if (!checked) {
        if (m_ratioGroup->checkedId() == -1) {
             m_ratioGroup->setExclusive(false);
             m_cropArea->setAspectRatio(0.0);
        }
        return;
    }
    m_ratioGroup->setExclusive(true);
    m_ratioGroup->button(id)->setChecked(true);

    switch(id) {
        case 0: m_cropArea->setAspectRatio(0.0); break;
        case 1: m_cropArea->setAspectRatio(1.0); break;
        case 2: m_cropArea->setAspectRatio(4.0/3.0); break;
        case 3: m_cropArea->setAspectRatio(16.0/9.0); break;
    }
}

void CropDialog::fitToWindow()
{
    double w_ratio = (double)m_scrollArea->width() / (m_sourceImage.width() + 10);
    double h_ratio = (double)m_scrollArea->height() / (m_sourceImage.height() + 10);
    double scale = qMin(w_ratio, h_ratio);
    m_cropArea->setScale(scale);
}

void CropDialog::oneToOne()
{
    m_cropArea->setScale(1.0);
}

void CropDialog::acceptCrop()
{
    QRect selection = m_cropArea->getSelection();
    if (!selection.isValid() || selection.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng vẽ một vùng chọn trước khi xuất.");
        return;
    }
    m_croppedImage = m_sourceImage.copy(selection);
    accept();
}
