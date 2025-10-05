#pragma once

#include <NXTableView.h>
#include "PWDef.h"

class PWHeaderView;
class PWTableViewModel;
class PWTableView : public NXTableView {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(PWHeaderView*, HeaderView)
    Q_PRIVATE_CREATE_D(PWTableViewModel*, Model)
    Q_PRIVATE_CREATE_D(QList<int>, ColumnWidthList)
    Q_PRIVATE_CREATE(WizConverter::Enums::ModuleType, ModuleType)
public:
    explicit PWTableView(QWidget* parent = nullptr);
    ~PWTableView();

    void setHeaderTextList(const QStringList& headerTextList);
    void setColumnWidthList(const QList<int>& columnWidthList);
    NXModelIndexWidget* createRangeWidget();
    NXModelIndexWidget* createSwitchWidget();
Q_SIGNALS:
    void checkIconClicked(const QModelIndex& index);
    void switchClicked(int index);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    
    //bool eventFilter(QObject* obj, QEvent* event) override;
private Q_SLOTS:
    void _onTableViewShow();
};