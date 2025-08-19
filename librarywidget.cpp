// librarywidget.cpp - Version 1.0
#include "librarywidget.h"
#include <QApplication>
#include <QDropEvent>

LibraryWidget::LibraryWidget(QWidget *parent) : QListWidget(parent)
{
    // Kích hoạt chức năng kéo-thả
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDropIndicatorShown(true);
}

void LibraryWidget::dropEvent(QDropEvent *event)
{
    // Gọi hàm gốc để Qt tự xử lý việc di chuyển item
    QListWidget::dropEvent(event);
    // Phát tín hiệu để báo cho cửa sổ chính rằng thứ tự đã thay đổi
    emit itemsMoved();
}
