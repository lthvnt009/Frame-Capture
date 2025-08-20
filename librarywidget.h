// librarywidget.h - Version 1.3
#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QListWidget>
#include <QMouseEvent>

class LibraryWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget *parent = nullptr);

signals:
    // YÊU CẦU: Signal mới
    void itemQuickExportRequested(QListWidgetItem *item);

protected:
    // YÊU CẦU: Ghi đè sự kiện
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // LIBRARYWIDGET_H
