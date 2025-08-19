// libraryitemdelegate.cpp - Version 1.0
#include "libraryitemdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

LibraryItemDelegate::LibraryItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // Lấy icon (thumbnail) và trạng thái check
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());

    // Vẽ thumbnail
    QRect itemRect = option.rect;
    QPixmap pixmap = icon.pixmap(itemRect.size());
    painter->drawPixmap(itemRect, pixmap);

    // Vẽ checkbox ở góc dưới bên trái
    QRect checkBoxRect = getCheckBoxRect(itemRect);
    QStyleOptionButton checkBoxOpt;
    checkBoxOpt.rect = checkBoxRect;
    checkBoxOpt.state = QStyle::State_Enabled;
    if (checkState == Qt::Checked) {
        checkBoxOpt.state |= QStyle::State_On;
    } else {
        checkBoxOpt.state |= QStyle::State_Off;
    }
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOpt, painter);

    painter->restore();
}

bool LibraryItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect checkBoxRect = getCheckBoxRect(option.rect);

        // Nếu click vào vùng checkbox, thay đổi trạng thái
        if (checkBoxRect.contains(mouseEvent->pos())) {
            Qt::CheckState currentState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            model->setData(index, newState, Qt::CheckStateRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QRect LibraryItemDelegate::getCheckBoxRect(const QRect &itemRect) const
{
    // Kích thước và vị trí của checkbox
    int size = 20;
    int margin = 5;
    return QRect(itemRect.left() + margin, itemRect.bottom() - size - margin, size, size);
}
