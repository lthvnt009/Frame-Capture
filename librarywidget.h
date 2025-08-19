// librarywidget.h - Version 1.2
#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QListWidget>

class LibraryWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget *parent = nullptr);

};

#endif // LIBRARYWIDGET_H
