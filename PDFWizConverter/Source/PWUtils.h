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

    inline QList<QRect> SplitRectHorizontally(const QRect& rect, int parts = 3)
    {
        if (parts <= 1)  return { rect };
        QList<QRect> result;
        result.reserve(parts);
        int partWidth = rect.width() / parts;

        for (int i = 0; i < parts; ++i) {
            int x = rect.left() + i * partWidth;
            int width = (i == parts - 1) ? rect.right() - x + 1 : partWidth;
            result.append(QRect(x, rect.top(), width, rect.height()));
        }

        return result;
    }

    inline QList<QRect> SplitRectVertically(const QRect& rect, int parts = 3)
    {
        if (parts <= 1)  return { rect };
        QList<QRect> result;
        result.reserve(parts);
        int partHeight = rect.height() / parts;

        for (int i = 0; i < parts; ++i) {
            int y = rect.top() + i * partHeight;
            int height = (i == parts - 1) ? rect.bottom() - y + 1 : partHeight;
            result.append(QRect(rect.left(), y, rect.width(), height));
        }

        return result;
    }
}