#pragma once
#include <NXDef.h>
#include <NXToolButton.h>
#include <QHash>

#include "PWDef.h"

class PWToolButton : public NXAdvancedToolButton {
    Q_OBJECT
public:
    struct MetaData {
        enum State {
            Normal,
            Hovered,
            Pressed,
            Unavailable
        };
        int FontPixelSize;
        QRect TextRect;
        QSize ButtonSize;
        QFont Font;
        QString Qss;
        QHash<State, QColor> TextColor;
        //QHash<ButtonState, QColor> _pBackgroundColor;
        //QHash<ButtonState, QColor> _pBorderColor;
    };
    Q_PRIVATE_CREATE_EX(const QString&, QString, ImagePath)
    Q_PRIVATE_CREATE_EX(const PWToolButton::MetaData&, PWToolButton::MetaData, ButtonMetaData)
public:
    explicit PWToolButton(QWidget* parent = nullptr);
    PWToolButton(const QString& text, const QString& imagePath,
        const PWToolButton::MetaData& metaData, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};