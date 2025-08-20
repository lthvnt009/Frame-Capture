// libraryitemdelegate.h - Version 1.1
#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

class LibraryItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit LibraryItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    // THÊM MỚI: Các hàm helper để tính toán vị trí
    QRect getPixmapRect(const QStyleOptionViewItem &option) const;
    QRect getCheckBoxRect(const QStyleOptionViewItem &option) const;
};

#endif // LIBRARYITEMDELEGATE_H
