// viewpanel.h - Version 1.3
#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include <QWidget>
#include <QList>
#include <QImage>
#include <QSize>
#include <QWheelEvent>

class ViewPanel : public QWidget
{
    Q_OBJECT

public:
    enum LayoutType { Grid, Vertical, Horizontal };

    explicit ViewPanel(QWidget *parent = nullptr);

    QImage getCompositedImage() const;

signals:
    void scaleChanged(int newScale);

public slots:
    void setImages(const QList<QImage> &images);
    void setLayoutType(LayoutType type);
    void setSpacing(int spacing);
    void setScale(int scalePercent);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QSize calculateTotalSize() const;
    int findBestColumnCount() const; // SỬA LỖI: Thêm khai báo hàm

    QList<QImage> m_images;
    LayoutType m_layoutType = Horizontal;
    int m_spacing = 5;
    double m_scale = 1.0;
};

#endif // VIEWPANEL_H
