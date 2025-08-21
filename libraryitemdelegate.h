// libraryitemdelegate.h - Version 1.2 (Constants added)
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
    QRect getPixmapRect(const QStyleOptionViewItem &option) const;
    QRect getCheckBoxRect(const QStyleOptionViewItem &option) const;

    // THÊM MỚI: Định nghĩa các hằng số để code dễ đọc và bảo trì
    static constexpr int CHECKBOX_SIZE = 16;
    static constexpr int CHECKBOX_MARGIN = 3;
    static constexpr int CHECKBOX_BG_PADDING = 2;
    static constexpr int CHECKBOX_BG_RADIUS = 2;
};

#endif // LIBRARYITEMDELEGATE_H
