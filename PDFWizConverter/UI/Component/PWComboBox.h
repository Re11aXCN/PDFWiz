#pragma once

#include <NXComboBox.h>

class QWheelEvent;
class PWComboBox : public NXComboBox {
    Q_OBJECT
    Q_PROPERTY_CREATE(bool, ClickSwitchEnabled)
    Q_PROPERTY_CREATE(bool, AllowRepeatClick)
public:
    explicit PWComboBox(QWidget* parent = nullptr);
    ~PWComboBox() override;

Q_SIGNALS:
    void itemClicked(int index);
    void itemClicked(const QString& text);
protected:
    void showPopup() override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};