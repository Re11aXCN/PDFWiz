#pragma once

#include <NXTableView.h>
#include "PWDef.h"

class PWHeaderView;
class PWTableViewModel;
class PWTableViewIconDelegate;
class PWTableView : public NXTableView {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(PWHeaderView*, HeaderView)
    Q_PRIVATE_CREATE_D(PWTableViewModel*, Model)
    Q_PRIVATE_CREATE_D(PWTableViewIconDelegate*, IconDelegate)
    Q_PRIVATE_CREATE_D(QList<int>, ColumnWidthList)
    Q_PRIVATE_CREATE_D(WizConverter::Enums::ModuleType, ModuleType)
    Q_PRIVATE_CREATE_Q_H(WizConverter::Enums::ModuleType, ModuleType)
public:
    explicit PWTableView(QWidget* parent = nullptr);
    ~PWTableView();

    void setHeaderTextList(const QStringList& headerTextList);
    void setColumnWidthList(const QList<int>& columnWidthList);
    NXModelIndexWidget* createLineEditRangeWidget();
    NXModelIndexWidget* createTwoLineEditRangeWidget();
    NXModelIndexWidget* createOneLineEditRangeWidget();
    NXModelIndexWidget* createSwitchWidget();
    QRect visualRectForRow(int row) const;
    QAbstractItemView::DropIndicatorPosition dropIndicatorPositionOverride() const;

Q_SIGNALS:
    void checkIconClicked(const QModelIndex& index);
    void switchClicked(int index);
    void swapRows(int row1, int row2);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    //bool eventFilter(QObject* obj, QEvent* event) override;
private :
    void _onTableViewShow();

    // 网格模式下滚动条更新
    void _updateGridViewScrollBar();
private:
    inline static std::once_flag _pOnceFlag;
};