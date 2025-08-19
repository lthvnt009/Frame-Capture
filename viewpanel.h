// viewpanel.h - Version 1.4
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

public slots:
    void setImages(const QList<QImage> &images);
    void setLayoutType(LayoutType type);
    void setSpacing(int spacing);
    void setScale(double newScale); // Thay đổi từ int sang double
    void fitToWindow();
    void setOneToOne();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QSize calculateTotalSize() const;
    int findBestColumnCount() const;

    QList<QImage> m_images;
    LayoutType m_layoutType = Horizontal;
    int m_spacing = 5;
    double m_scale = 1.0;
};

#endif // VIEWPANEL_H
