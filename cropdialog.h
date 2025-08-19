// cropdialog.h - Version 2.6
#ifndef CROPDIALOG_H
#define CROPDIALOG_H

#include <QDialog>
#include <QImage>
#include <QRect>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainterPath>
#include <QUndoStack>

class CropArea : public QWidget
{
    Q_OBJECT
public:
    enum Handle { None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, Move };

    explicit CropArea(QWidget *parent = nullptr);
    QRect getSelection() const;
    void setImage(const QImage &image); // Cần public để cập nhật ảnh sau khi cắt

public slots:
    void setAspectRatio(double ratio);
    void setScale(double newScale);
    void clearSelection();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateCursor(const QPointF &pos);
    void resizeSelection(const QPointF &pos);
    Handle getHandleAt(const QPointF &pos) const;
    QRectF getHandleRect(Handle handle) const;

    QImage m_image;
    QRectF m_selectionRect;
    Handle m_currentHandle = None;
    QPointF m_dragStartPos;
    double m_aspectRatio = 0.0;
    double m_scale = 1.0;
};

class QScrollArea;
class QButtonGroup;
class QPushButton;
class QAction;

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CropDialog(const QImage &image, QWidget *parent = nullptr);
    QImage getFinalImage() const; // Đổi tên để rõ nghĩa hơn

signals:
    void exportImageRequested(const QImage &image);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onAspectRatioChanged(int id, bool checked);
    void fitToWindow();
    void oneToOne();
    void applyCrop(); // Slot mới cho phím Enter
    void exportImage(); // Slot mới cho nút "Xuất ảnh"

private:
    void setupUi();

    CropArea *m_cropArea;
    QScrollArea *m_scrollArea;
    QButtonGroup *m_ratioGroup;
    QImage m_currentImage; // Ảnh đang được chỉnh sửa

    // Chức năng Undo/Redo cho thao tác cắt
    QUndoStack *m_undoStack;
    QAction *m_undoAction;
    QAction *m_redoAction;
};

#endif // CROPDIALOG_H
