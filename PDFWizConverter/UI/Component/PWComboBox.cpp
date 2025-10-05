#include "PWComboBox.h"
#include <QWheelEvent>
#include <QAbstractItemView>
PWComboBox::PWComboBox(QWidget* parent)
    : NXComboBox(parent)
    , _pClickSwitchEnabled{true}
    , _pAllowRepeatClick{false}
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
    if (view()) {
        view()->viewport()->installEventFilter(this);
    }
    NXComboBox::showPopup();
}

void PWComboBox::wheelEvent(QWheelEvent* event)
{
    // 禁止滚轮事件传递到父级
    event->ignore();
}

bool PWComboBox::eventFilter(QObject* watched, QEvent* event)
{
    if (!_pClickSwitchEnabled && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        const QModelIndex& index = view()->indexAt(mouseEvent->pos());
        
        if (index.isValid()) {
            if (currentIndex() != index.row() || _pAllowRepeatClick) {
                Q_EMIT itemClicked(index.row());
                Q_EMIT itemClicked(itemText(index.row()));
            }
            // 隐藏弹出框
            hidePopup();

            return true; // 事件已处理，阻止默认行为
        }
    }

    return QComboBox::eventFilter(watched, event);
}