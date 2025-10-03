#include "PWTableView.h"
#include <QScrollBar>
#include <QPainter>
#include "PWHeaderView.h"
#include "PWTableViewModel.h"

namespace Scope::Utils {
    struct HeaderViewMetadata {
        QStringList TextList;
        QList<int> ColumnList;
    };
}

PWTableView::PWTableView(QWidget* parent)
    : NXTableView(parent)
{
    _pHeaderView = new PWHeaderView(Qt::Horizontal, this);
    _pModel = new PWTableViewModel(this);
    setModel(_pModel);
    setHorizontalHeader(_pHeaderView);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setFixedHeight(40);
    verticalHeader()->setHidden(true);
    verticalHeader()->setMinimumSectionSize(50);
    horizontalScrollBar()->setVisible(false);
    QObject::connect(this, &NXTableView::tableViewShow, this,  &PWTableView::_onTableViewShow);
}

PWTableView::~PWTableView()
{
}

void PWTableView::paintEvent(QPaintEvent* event)
{
    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    NXTableView::paintEvent(event);
}

void PWTableView::_onTableViewShow()
{

}

