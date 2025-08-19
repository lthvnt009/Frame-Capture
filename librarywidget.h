// librarywidget.h - Version 1.0
#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QListWidget>

class LibraryWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget *parent = nullptr);

signals:
    void itemsMoved();

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // LIBRARYWIDGET_H
