#pragma once

#include <QFrame>
#include <NXProperty.h>

class QLabel;
class PWTableMaskWidget : public QFrame {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(QLabel*, ImageLabel)
    Q_PROPERTY_CREATE_Q_EX_H(const QPixmap&, QPixmap, Mask)
public:
    explicit PWTableMaskWidget(QWidget* parent = nullptr);
    explicit PWTableMaskWidget(const QPixmap& mask, QWidget* parent = nullptr);
    ~PWTableMaskWidget() = default;
Q_SIGNALS:
    void pressed();
    void released();
protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};