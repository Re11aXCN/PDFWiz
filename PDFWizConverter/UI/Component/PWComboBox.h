#pragma once

#include <NXComboBox.h>

class QWheelEvent;
class PWComboBox : public NXComboBox {
    Q_OBJECT
public:
    explicit PWComboBox(QWidget* parent = nullptr);
    ~PWComboBox() override;

protected:
    void showPopup() override;
    void wheelEvent(QWheelEvent* event) override;
};