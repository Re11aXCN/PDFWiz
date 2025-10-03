#include "PWHeaderView.h"

#include <QPainter>
#include <QMouseEvent>

#include "PWUtils.h"
#include "PWToolTip.h"
using namespace WizConverter;
PWHeaderView::PWHeaderView(Qt::Orientation orientation, QWidget* parent)
	: QHeaderView(orientation, parent), _pIsClearRectHovered(false)
{
    setMouseTracking(true);
    setSectionsClickable(true);
    setSectionsMovable(true);   // 允许移动列
    setDragEnabled(true);       // 启用拖拽
}

void PWHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
	QHeaderView::paintSection(painter, rect, logicalIndex);
//#if(gImage2PdfIsToggle) return;
    if (logicalIndex == count() - 1)
    {
        int height = font().pixelSize();
        QRect iconRect = Utils::GetAlignCenter(
            QRect(sectionPosition(logicalIndex), 0, sectionSize(logicalIndex), this->height()),
            QSize(height * 2 + 6, height + 4)
        );

        painter->fillRect(iconRect, _pIsClearRectHovered ? QColor(0xA3, 0xC7, 0xD8) : QColor(0, 0, 0, 0));
        painter->drawText(iconRect, Qt::AlignCenter, "删除");
        painter->setPen(QColor(0x54, 0x8E, 0xF8));
        painter->drawLine(iconRect.left() + 3, iconRect.bottom() - 1, iconRect.right() - 3, iconRect.bottom() - 1);
    }
}

void PWHeaderView::mouseMoveEvent(QMouseEvent* event)
{
    int section = logicalIndexAt(event->pos());
    if (section == 0 || section == 1)
    {
        int height = font().pixelSize();
        QRect iconRect = Utils::GetAlignLeft(
            QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
            section == 0 ? QSize(23, 23) : QSize(height * 2, height)
        );
        section == 0 ? iconRect.adjust(16, 5, 11, -1) : iconRect.adjust(5, 0, 5, 0);

        iconRect.contains(event->pos()) 
            ? parentWidget()->setCursor(Qt::PointingHandCursor)
            : parentWidget()->unsetCursor();
    }
    else if (section == count() - 1)
    {
        int height = font().pixelSize();
        QRect iconRect = Utils::GetAlignCenter(
            QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
            QSize(height * 2, height));

        QRegion region(iconRect);
        if (iconRect.contains(event->pos()))
        {
            parentWidget()->setCursor(Qt::PointingHandCursor);
            if (!_pIsClearRectHovered)
            {
                _pIsClearRectHovered = true;
                PWToolTip::showToolTip(QStringLiteral("清空当前列表"), viewport()->mapToGlobal(iconRect.bottomLeft() + QPoint(0, 1)));
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
        if (section == 0 || section == 1)
        {
            int height = font().pixelSize();
            QRect iconRect = Utils::GetAlignLeft(
                QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
                section == 0 ? QSize(23, 23) : QSize(height * 2, height)
            );
            section == 0 ? iconRect.adjust(16, 5, 11, -1) : iconRect.adjust(5, 0, 5, 0);

            if (iconRect.contains(event->pos())) Q_EMIT selectAllData();
        }
        else if (section == count() - 1)
        {
            int height = font().pixelSize();
            QRect iconRect = Utils::GetAlignCenter(
                QRect(sectionPosition(section), 0, sectionSize(section), this->height()),
                QSize(height * 2, height)
            );
            if (iconRect.contains(event->pos())) Q_EMIT deleteSelectedData();
        }
    }
    QHeaderView::mouseReleaseEvent(event);
}

void PWHeaderView::leaveEvent(QEvent* event)
{
    QHeaderView::leaveEvent(event);
}
