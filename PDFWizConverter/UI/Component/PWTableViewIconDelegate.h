#pragma once

#include <QStyledItemDelegate>
#include <NXDef.h>

class PWTableView;
class PWTableViewIconDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum IconRole {
        Run,
        OpenFile,
        OpenFolder,
        RemoveFile,
    };
    explicit PWTableViewIconDelegate(QObject* parent = nullptr);
    ~PWTableViewIconDelegate();

Q_SIGNALS:
    void iconClicked(IconRole iconType, const QModelIndex& index);

protected:
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
    // 绘制相关方法
    void _paintGridViewMode(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void _paintListViewMode(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    // 事件处理相关方法
    bool _handleGridViewEditorEvent(QEvent* event, const QStyleOptionViewItem& option, const QModelIndex& index, const PWTableView* widget);
    bool _handleListViewEditorEvent(QEvent* event, const QStyleOptionViewItem& option, const QModelIndex& index, const PWTableView* widget);

    // 工具方法
    QPoint _getAdjustedMousePosition(const QStyleOptionViewItem& option) const;
    QList<QRect> _getIconCellRects(const QStyleOptionViewItem& option, const PWTableView* widget, const QModelIndex& index) const;

private:
    inline static std::once_flag _pOnceFlag;
    inline static QSize _pIconSize;
    QImage _pCircleXImage;
    QImage _pDoubleClickHintImage;
};