#pragma once

#include <QRect>
namespace WizConverter::Utils {
    inline QRect GetAlignLeft(const QRect& cellRect, const QSize& iconSize)
    {
        return QRect(cellRect.x(),
            cellRect.y() + ((cellRect.height() - iconSize.height()) >> 1),
            iconSize.width(),
            iconSize.height());
    }

    inline QRect GetAlignCenter(const QRect& cellRect, const QSize& iconSize)
    {
        return QRect(cellRect.x() + ((cellRect.width() - iconSize.width()) >> 1),
            cellRect.y() + ((cellRect.height() - iconSize.height()) >> 1),
            iconSize.width(),
            iconSize.height());
    }

    inline QRect GetAlignRight(const QRect& cellRect, const QSize& iconSize)
    {
        return QRect(cellRect.x() + (cellRect.width() - iconSize.width()),
            cellRect.y() + ((cellRect.height() - iconSize.height()) >> 1),
            iconSize.width(),
            iconSize.height());
    }
}