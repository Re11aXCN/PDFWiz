#include "PWHeaderView.h"

#include <QPainter>
#include <QMouseEvent>

#include "PWDef.h"
#include "PWUtils.h"
#include "PWToolTip.h"

PWHeaderView::PWHeaderView(Qt::Orientation orientation, QWidget* parent)
	: QHeaderView(orientation, parent), _pIsClearRectHovered(false)
{
    setIconSize(TABLE_VIEW_CHECKICON_SIZE);
    setMouseTracking(true);
    setSectionsClickable(true);
    setSectionsMovable(true);   // 允许移动列
    setDragEnabled(true);       // 启用拖拽
}

void PWHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
//#if(gImage2PdfIsToggle) return;

    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == count() - 1 || (logicalIndex == 3 && model()->property("IsGridViewMode").toBool()))
    {
        int height = font().pixelSize();
        QRect iconRect = WizConverter::Utils::GetAlignCenter(
            QRect(sectionPosition(logicalIndex), 0, sectionSize(logicalIndex), this->height()),
            QSize(height * 2 + 8, height + 8)
        );
        painter->fillRect(iconRect, _pIsClearRectHovered ? QColor(0xA3, 0xC7, 0xD8) : QColor(0, 0, 0, 0));
        QVariant textColor = model()->headerData(logicalIndex, Qt::Horizontal, Qt::ForegroundRole);
        textColor.isValid() ? painter->setPen(textColor.value<QColor>()) : painter->setPen(Qt::black);
        painter->drawText(iconRect, Qt::AlignCenter, "删除");
        painter->setPen(QPen{ QColor(0x54, 0x8E, 0xF8), 2 });
        painter->drawLine(iconRect.left() + 3, iconRect.bottom() - 1, iconRect.right() - 3, iconRect.bottom() - 1);
    }
}

void PWHeaderView::mouseMoveEvent(QMouseEvent* event)
{
    int section = logicalIndexAt(event->pos());
    if (section == 0 || (section == 1 && !model()->property("IsGridViewMode").toBool()))
    {
        int height = font().pixelSize();
        QRect iconRect = WizConverter::Utils::GetAlignLeft(
            QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
            section == 0 ? QSize(23, 23) : QSize(height * 2, height)
        );
        section == 0 ? iconRect.adjust(TABLE_VIEW_CHECKICON_ADJUST) : iconRect.adjust(5, 0, 5, 0);

        iconRect.contains(event->pos()) 
            ? parentWidget()->setCursor(Qt::PointingHandCursor)
            : parentWidget()->unsetCursor();
    }
    else if (section == count() - 1 || (section == 3 && model()->property("IsGridViewMode").toBool()))
    {
        int height = font().pixelSize();
        QRect iconRect = WizConverter::Utils::GetAlignCenter(
            QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
            QSize(height * 2, height));

        QRegion region(iconRect);
        if (iconRect.contains(event->pos()))
        {
            parentWidget()->setCursor(Qt::PointingHandCursor);
            if (!_pIsClearRectHovered)
            {
                _pIsClearRectHovered = true;
                PWToolTip::showToolTip(QStringLiteral("清空当前已选择"), viewport()->mapToGlobal(iconRect.bottomLeft() + QPoint(0, 1)));
            }
            update(region);
        }
        else
        {
            parentWidget()->unsetCursor();
            _pIsClearRectHovered = false;
            PWToolTip::hideToolTip();
            update(region);
        }
    }
    QHeaderView::mouseMoveEvent(event);
}

void PWHeaderView::mousePressEvent(QMouseEvent* event)
{
    QHeaderView::mousePressEvent(event);
}

void PWHeaderView::mouseReleaseEvent(QMouseEvent* event)
{
    //#if (gImage2PdfIsToggle) return;

    if (event->button() == Qt::LeftButton)
    {
        int section = logicalIndexAt(event->pos());
        if (section == 0 || (section == 1 && !model()->property("IsGridViewMode").toBool()))
        {
            int height = font().pixelSize();
            QRect iconRect = WizConverter::Utils::GetAlignLeft(
                QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
                section == 0 ? QSize(23, 23) : QSize(height * 2, height)
            );
            section == 0 ? iconRect.adjust(TABLE_VIEW_CHECKICON_ADJUST) : iconRect.adjust(5, 0, 5, 0);

            if (iconRect.contains(event->pos())) Q_EMIT selectAllRows();
        }
        else if (section == count() - 1)
        {
            int height = font().pixelSize();
            QRect iconRect = WizConverter::Utils::GetAlignCenter(
                QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
                QSize(height * 2, height)
            );
            if (iconRect.contains(event->pos())) Q_EMIT removeSelectedRows();
        }
    }
    QHeaderView::mouseReleaseEvent(event);
}

void PWHeaderView::leaveEvent(QEvent* event)
{
    _pIsClearRectHovered = false;
    PWToolTip::hideToolTip();
    QHeaderView::leaveEvent(event);
}
