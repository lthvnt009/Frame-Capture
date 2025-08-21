// libraryitemdelegate.cpp - Version 1.3 (Magic Numbers Removed)
#include "libraryitemdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QIcon>

LibraryItemDelegate::LibraryItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QRect LibraryItemDelegate::getPixmapRect(const QStyleOptionViewItem &option) const
{
    QIcon icon = qvariant_cast<QIcon>(option.index.data(Qt::DecorationRole));
    // Giảm padding để ảnh lớn hơn một chút
    QRect contentRect = option.rect.adjusted(1, 1, -1, -1);
    QPixmap pixmap = icon.pixmap(contentRect.size(), QIcon::Normal, QIcon::On);
    
    int pixmapX = contentRect.x() + (contentRect.width() - pixmap.width()) / 2;
    int pixmapY = contentRect.y() + (contentRect.height() - pixmap.height()) / 2;
    return QRect(pixmapX, pixmapY, pixmap.width(), pixmap.height());
}

QRect LibraryItemDelegate::getCheckBoxRect(const QStyleOptionViewItem &option) const
{
    QRect pixmapRect = getPixmapRect(option);
    // THAY ĐỔI: Sử dụng hằng số thay vì "magic numbers"
    int size = CHECKBOX_SIZE;
    int margin = CHECKBOX_MARGIN;
    return QRect(pixmapRect.left() + margin, pixmapRect.bottom() - size - margin, size, size);
}


void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    
    // Vẽ thumbnail
    QRect pixmapRect = getPixmapRect(option);
    painter->drawPixmap(pixmapRect.topLeft(), icon.pixmap(pixmapRect.size()));

    // Vẽ checkbox
    QRect checkBoxRect = getCheckBoxRect(option);
    
    // Vẽ nền mờ cho checkbox để dễ nhìn
    painter->setBrush(QColor(0, 0, 0, 100));
    painter->setPen(Qt::NoPen);
    // THAY ĐỔI: Sử dụng hằng số
    painter->drawRoundedRect(checkBoxRect.adjusted(-CHECKBOX_BG_PADDING, -CHECKBOX_BG_PADDING, CHECKBOX_BG_PADDING, CHECKBOX_BG_PADDING), CHECKBOX_BG_RADIUS, CHECKBOX_BG_RADIUS);

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
        QRect checkBoxRect = getCheckBoxRect(option);

        if (checkBoxRect.contains(mouseEvent->pos())) {
            Qt::CheckState currentState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            model->setData(index, newState, Qt::CheckStateRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
