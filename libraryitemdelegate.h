// libraryitemdelegate.h - Version 1.0
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
    QRect getCheckBoxRect(const QRect &itemRect) const;
};

#endif // LIBRARYITEMDELEGATE_H
