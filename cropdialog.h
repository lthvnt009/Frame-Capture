// cropdialog.h - Version 2.8
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

// Forward declarations
class CropArea;
class QScrollArea;
class QButtonGroup;
class QPushButton;
class QAction;
class QLineEdit;
class QSpinBox;
class QRadioButton; // SỬA LỖI: Thêm forward declaration

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CropDialog(const QImage &image, QWidget *parent = nullptr);
    QImage getFinalImage() const;

signals:
    void exportImageRequested(const QImage &image);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onAspectRatioChanged(int id, bool checked);
    void updateCustomAspectRatio();
    void fitToWindow();
    void oneToOne();
    void applyCrop();
    void exportImage();

private:
    void setupUi();

    CropArea *m_cropArea;
    QScrollArea *m_scrollArea;
    QButtonGroup *m_ratioGroup;
    QImage m_currentImage;

    QWidget *m_customRatioWidget;
    QRadioButton *m_customRatioRadio;
    QSpinBox *m_customWidthSpinBox;
    QSpinBox *m_customHeightSpinBox;

    QUndoStack *m_undoStack;
    QAction *m_undoAction;
    QAction *m_redoAction;
};

#endif // CROPDIALOG_H
