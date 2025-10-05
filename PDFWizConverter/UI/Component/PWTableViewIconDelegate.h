#pragma once

#include <QStyledItemDelegate>
#include <NXDef.h>

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
    inline static std::once_flag _pOnceFlag;
    inline static QSize _pIconSize;
};