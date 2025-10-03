#pragma once

#include <NXTableView.h>
#include <NXProperty.h>

class PWHeaderView;
class PWTableViewModel;
class PWTableView : public NXTableView {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(PWHeaderView*, HeaderView)
    Q_PRIVATE_CREATE_D(PWTableViewModel*, Model)
public:
    explicit PWTableView(QWidget* parent = nullptr);
    ~PWTableView();

Q_SIGNALS:

protected:
    void paintEvent(QPaintEvent* event) override;
    //void mouseReleaseEvent(QMouseEvent* event) override;
    //void mouseMoveEvent(QMouseEvent* event) override;
    //void leaveEvent(QEvent* event) override;
    //
    //bool eventFilter(QObject* obj, QEvent* event) override;
private Q_SLOTS:
    void _onTableViewShow();
};