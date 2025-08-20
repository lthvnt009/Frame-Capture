// librarywidget.cpp - Version 1.4
#include "librarywidget.h"
#include <QApplication>
#include <QDropEvent>
#include <QListView>
#include <QMouseEvent>

LibraryWidget::LibraryWidget(QWidget *parent) : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setMovement(QListView::Static);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSpacing(5); // Sẽ được ghi đè trong mainwindow
}

// YÊU CẦU: Triển khai hàm xử lý sự kiện mới
void LibraryWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QListWidgetItem *item = itemAt(event->pos());
        if (item) {
            emit itemQuickExportRequested(item);
            event->accept();
            return;
        }
    }
    // Chuyển cho lớp cơ sở xử lý các sự kiện khác (vd: double left-click)
    QListWidget::mouseDoubleClickEvent(event);
}
