#include "PWToolButton.h"
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>

PWToolButton::PWToolButton(QWidget* parent)
    : NXAdvancedToolButton(parent)
{

}

PWToolButton::PWToolButton(const QString& text, const QString& imagePath
    , const PWToolButton::MetaData& metaData, QWidget* parent)
    : PWToolButton(parent)
{
    _pImagePath = imagePath;
    _pButtonMetaData = metaData;

    setCheckable(true); 
    setCursor(Qt::PointingHandCursor);
    setText(text);

    setFixedSize(_pButtonMetaData.ButtonSize);
    setStyleSheet(_pButtonMetaData.Qss);
    _pButtonMetaData.Font.setPixelSize(_pButtonMetaData.FontPixelSize);
    setFont(_pButtonMetaData.Font);
}

void PWToolButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    QPixmap pixmap(_pImagePath);
    QRect sourceRect;
    if (!isEnabled()) {
        painter.setPen(_pButtonMetaData.TextColor[PWToolButton::MetaData::Unavailable]);
        sourceRect = QRect(QPoint(_pButtonMetaData.ButtonSize.width() * 3, 0), _pButtonMetaData.ButtonSize);// 图片的第四部分
    }
    else if (isChecked() || isDown()) {
        painter.setPen(_pButtonMetaData.TextColor[PWToolButton::MetaData::Pressed]);
        sourceRect = QRect(QPoint(_pButtonMetaData.ButtonSize.width() * 2, 0), _pButtonMetaData.ButtonSize);// 图片的第三部分
    }
    else if (underMouse()) {
        painter.setPen(_pButtonMetaData.TextColor[PWToolButton::MetaData::Hovered]);
        sourceRect = QRect(QPoint(_pButtonMetaData.ButtonSize.width() * 1, 0), _pButtonMetaData.ButtonSize);// 图片的第二部分
    }
    else {
        painter.setPen(_pButtonMetaData.TextColor[PWToolButton::MetaData::Normal]);
        sourceRect = QRect(QPoint(0, 0), _pButtonMetaData.ButtonSize);                                // 图片的第一部分
    }
    painter.drawPixmap(rect(), pixmap, sourceRect);
    painter.drawText(_pButtonMetaData.TextRect, Qt::AlignCenter, text());
}

void PWToolButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setChecked(!isChecked());
    }
    NXAdvancedToolButton::mousePressEvent(event);
}

void PWToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    NXAdvancedToolButton::mouseReleaseEvent(event);
}
