#include "PWTableViewIconDelegate.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHeaderView>
#include <NXTheme.h>

#include "PWUtils.h"
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
    bool isSecondLastColumn = (index.column() == (headerColumnCount - 2));
    int iconCount = isSecondLastColumn ? 3 : 1;
    
    QList<QRect> cellRects = WizConverter::Utils::SplitRectHorizontally(isSecondLastColumn ? option.rect.adjusted(10, 0, -10, 0) : option.rect, iconCount);

    QPoint localMousePos = option.widget->mapFromGlobal(QCursor::pos());
    localMousePos.setY(localMousePos.y() - 45);     // 修正鼠标位置 1.5 倍表头的height

    QFont font("NXAwesome");
    // 计算icon大小必须先设置NXAwesome字体,否则 DirectWrite: CreateFontFaceFromHDC() failed
    std::call_once(_pOnceFlag, [&]() {
        painter->setFont(font);
        _pIconSize = QSize(painter->fontMetrics().horizontalAdvance(icons[0]) + 10, painter->fontMetrics().height() + 8);
        });
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    for (int i = 0; i < iconCount; ++i) {
        QStyleOptionToolButton buttonOption;
        buttonOption.rect = WizConverter::Utils::GetAlignCenter(cellRects[i], _pIconSize);
        buttonOption.state = QStyle::State_Enabled;

        /*if (option.state & QStyle::State_Sunken) {
            buttonOption.state |= QStyle::State_Sunken;
        }*/
        if (buttonOption.rect.contains(localMousePos)) {
            buttonOption.state |= QStyle::State_MouseOver;
            painter->fillRect(buttonOption.rect, QColor(163, 199, 216));
            font.setPixelSize(17); // 悬停时放大
        }
        else {
            font.setPixelSize(16); // 正常大小
        }
        painter->setFont(font);

        painter->setPen(!viewOption.state.testFlag(QStyle::State_Enabled) ?
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicTextDisable) :
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicText));
        painter->drawText(buttonOption.rect, Qt::AlignCenter, icons[isSecondLastColumn ? i : 3]);
    }
    painter->restore();

    QStyledItemDelegate::paint(painter, viewOption, index);
}

QSize PWTableViewIconDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

bool PWTableViewIconDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
    {
        viewOption.state &= ~QStyle::State_HasFocus;
    }
    const PWTableView* widget = qobject_cast<const PWTableView*>(viewOption.widget);
    int headerColumnCount = widget->horizontalHeader()->count();
    bool isSecondLastColumn = (index.column() == (headerColumnCount - 2));
    int iconCount = isSecondLastColumn ? 3 : 1;

    QList<QRect> cellRects = WizConverter::Utils::SplitRectHorizontally(isSecondLastColumn ? option.rect.adjusted(10, 0, -10, 0) : option.rect, iconCount);

    QPoint localMousePos = option.widget->mapFromGlobal(QCursor::pos());
    localMousePos.setY(localMousePos.y() - 45);     // 修正鼠标位置 1.5 倍表头的height

    for (int i = 0; i < iconCount; ++i) {
        QStyleOptionToolButton buttonOption;
        buttonOption.rect = WizConverter::Utils::GetAlignCenter(cellRects[i], _pIconSize);
        buttonOption.state = QStyle::State_Enabled;

        if (buttonOption.rect.contains(localMousePos)) {
            if (event->type() == QEvent::MouseButtonRelease) {
                Q_EMIT iconClicked(isSecondLastColumn ? static_cast<IconRole>(i) : IconRole::RemoveFile, index);
                widget->viewport()->setCursor(Qt::PointingHandCursor);
                return true;
            }
            else if (event->type() == QEvent::MouseMove) {
                widget->viewport()->setCursor(Qt::PointingHandCursor);
                return true;
            }
        }
        else {
            widget->viewport()->unsetCursor();
        }
    }
    return false;
}

bool PWTableViewIconDelegate::eventFilter(QObject* obj, QEvent* event)
{
    return QStyledItemDelegate::eventFilter(obj, event);
}
