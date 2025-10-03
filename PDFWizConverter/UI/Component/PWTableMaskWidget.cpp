#include "PWTableMaskWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

PWTableMaskWidget::PWTableMaskWidget(QWidget* parent)
    : QFrame(parent)
{
    //setFixedSize(980, 335);
    setMouseTracking(true);
    setStyleSheet("QFrame{border: 1px solid #DDDDDD; background-color: #F7F5F6;}");

    QLabel* tipLabel1 = new QLabel("点击添加文件", this);
    tipLabel1->setFixedSize(18 * 6, 18);
    tipLabel1->setAlignment(Qt::AlignCenter);
    tipLabel1->setStyleSheet("QLabel{border:none;color: rgb(84,142,247);font-size: 18px;}");
    QLabel* tipLabel2 = new QLabel(" 或 ", this);
    tipLabel2->setFixedSize(18 * 1, 18);
    tipLabel2->setAlignment(Qt::AlignCenter);
    tipLabel2->setStyleSheet("QLabel{border:none;color: rgb(153,153,153);font-size: 18px;}");
    QLabel* tipLabel3 = new QLabel("拖拽至此区域", this);
    tipLabel3->setFixedSize(18 * 6, 18);
    tipLabel3->setAlignment(Qt::AlignCenter);
    tipLabel3->setStyleSheet("QLabel{border:none;color: rgb(84,142,247);font-size: 18px;}");

    _pImageLabel = new QLabel(this);
    _pImageLabel->setStyleSheet("QLabel{border:none;}");
    _pImageLabel->setPixmap(QPixmap::fromImage(QImage{}));
    _pImageLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* tipLabelLayout = new QHBoxLayout();
    tipLabelLayout->setContentsMargins(0, 0, 0, 0);
    tipLabelLayout->setSpacing(0);
    tipLabelLayout->addWidget(tipLabel1);
    tipLabelLayout->addWidget(tipLabel2);
    tipLabelLayout->addWidget(tipLabel3);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(_pImageLabel);
    layout->addStretch();
    layout->addLayout(tipLabelLayout);
    layout->addStretch();
}

PWTableMaskWidget::PWTableMaskWidget(const QPixmap& mask, QWidget* parent)
    : QFrame(parent)
{
    _pImageLabel->setPixmap(mask);
}

void PWTableMaskWidget::setMask(const QPixmap& mask) {
    _pImageLabel->setPixmap(mask);
    update();
    Q_EMIT pMaskChanged();
}

QPixmap PWTableMaskWidget::getMask() const {
    return _pImageLabel->pixmap();
}

void PWTableMaskWidget::enterEvent(QEnterEvent* event)
{
    setCursor(Qt::PointingHandCursor);
    QFrame::enterEvent(event);
}

void PWTableMaskWidget::leaveEvent(QEvent* event)
{
    setCursor(Qt::ArrowCursor);
    QFrame::leaveEvent(event);
}

void PWTableMaskWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT pressed();
    }
    QFrame::mousePressEvent(event);
}

void PWTableMaskWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT released();
    }
    QFrame::mouseReleaseEvent(event);
}
