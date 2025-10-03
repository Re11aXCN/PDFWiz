#include "PWTableViewIconDelegate.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHeaderView>

#include "PWTableView.h"
PWTableViewIconDelegate::PWTableViewIconDelegate(QObject* parent)
    : QStyledItemDelegate{ parent }
{

}

PWTableViewIconDelegate::~PWTableViewIconDelegate()
{
}

void PWTableViewIconDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static QChar icons[] = {
        QChar((unsigned short)NXIconType::Play),
        QChar((unsigned short)NXIconType::FileLines),
        QChar((unsigned short)NXIconType::FolderOpen),
        QChar((unsigned short)NXIconType::TrashCan)
    };
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);

    if (option.state.testFlag(QStyle::State_HasFocus))
    {
        viewOption.state &= ~QStyle::State_HasFocus;
    }
    const PWTableView* widget = qobject_cast<const PWTableView*>(viewOption.widget);
    int headerColumnCount = widget->horizontalHeader()->count();
    int iconCount = (index.column() == (headerColumnCount - 2)) ? 3 : 1;
    QFont font = widget->font();
    font.setPixelSize(16);
    QPoint localMousePos = option.widget->mapFromGlobal(QCursor::pos());
    localMousePos.setY(localMousePos.y() - 45);     // 修正鼠标位置 1.5 倍表头的height
    auto calculateIconRectFunc = []() {};
    painter->save();
    painter->setFont(font);

    painter->restore();

    QStyledItemDelegate::paint(painter, viewOption, index);
}

QSize PWTableViewIconDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize();
}

bool PWTableViewIconDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

bool PWTableViewIconDelegate::eventFilter(QObject* obj, QEvent* event)
{
    return false;
}
