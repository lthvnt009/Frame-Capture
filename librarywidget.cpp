// librarywidget.cpp - Version 1.3
#include "librarywidget.h"
#include <QApplication>
#include <QDropEvent>
#include <QListView>

LibraryWidget::LibraryWidget(QWidget *parent) : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setMovement(QListView::Static);

    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSpacing(5);
}
