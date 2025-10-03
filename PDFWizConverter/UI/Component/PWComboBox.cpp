#include "PWComboBox.h"
#include <QWheelEvent>

PWComboBox::PWComboBox(QWidget* parent)
    : NXComboBox(parent)
{
}

PWComboBox::~PWComboBox()
{
}

void PWComboBox::showPopup()
{
    // 保持父级的焦点
    QWidget* parent = this->parentWidget();
    if (parent) {
        parent->setFocus();
    }
    NXComboBox::showPopup(); // 调用基类的 showPopup
}

void PWComboBox::wheelEvent(QWheelEvent* event)
{
    // 禁止滚轮事件传递到父级
    event->ignore();
}
