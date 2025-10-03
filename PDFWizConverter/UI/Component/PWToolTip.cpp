#include "PWToolTip.h"
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QGuiApplication>
#include <QScreen>

static const QString WidgetTipStyle = QStringLiteral(R"(QLabel{})");
static const QString TextTipStyle = QStringLiteral(R"(QLabel{ color: #333333; background-color: #FFFFE0; border: 1px solid #CCCCCC; border-radius: 3px; })");

constexpr int WIDGEPWTIP_MARGIN = 10;
constexpr int TEXPWTIP_MARGIN = 2;
constexpr int MAX_TOOLTIP_WIDTH = 600;

PWToolTip::PWToolTip(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint/* | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint*/);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    _pContent = new QLabel(this);
    _pContent->setContentsMargins(0, 0, 0, 0);
    _pContent->setAlignment(Qt::AlignCenter);
    _pContent->setTextFormat(Qt::RichText);
    _pContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _pContent->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(_pContent);
}

void PWToolTip::hideToolTip()
{
    if (_pCurrentToolTip) {
        _pCurrentToolTip->_pContent->clear();
        _pCurrentToolTip->setVisible(false);
    }
}

void PWToolTip::showToolTip(const QString& text, const QPoint& pos, TipStyle style)
{
    std::call_once(_pFlag, [] { _pCurrentToolTip.reset(new PWToolTip()); });

    _pCurrentToolTip->_pTipStyle = style;
    switch (style)
    {
    case PWToolTip::WidgetTip:
        qobject_cast<QVBoxLayout*>(_pCurrentToolTip->layout())->setContentsMargins(WIDGEPWTIP_MARGIN, WIDGEPWTIP_MARGIN, WIDGEPWTIP_MARGIN, WIDGEPWTIP_MARGIN);
        _pCurrentToolTip->_pContent->setStyleSheet(WidgetTipStyle);
        break;
    case PWToolTip::TextTip:
        qobject_cast<QVBoxLayout*>(_pCurrentToolTip->layout())->setContentsMargins(TEXPWTIP_MARGIN, TEXPWTIP_MARGIN, TEXPWTIP_MARGIN, TEXPWTIP_MARGIN);
        _pCurrentToolTip->_pContent->setStyleSheet(TextTipStyle);
        break;
    default:
        qobject_cast<QVBoxLayout*>(_pCurrentToolTip->layout())->setContentsMargins(0, 0, 0, 0);
        break;
    }
    _pCurrentToolTip->updateGeometry();
    _pCurrentToolTip->_updateContent(text, pos);
    _pCurrentToolTip->setVisible(true);
}

void PWToolTip::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (_pCurrentToolTip->_pTipStyle == PWToolTip::WidgetTip)
    {
        // 绘制渐变阴影
        QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
        gradient.setColorAt(0, QColor(0x00, 0x66, 0xFF, 0x77));
        gradient.setColorAt(1, QColor(0xFF, 0x66, 0x00, 0x77));

        painter.fillRect(rect(), gradient);
        painter.setBrush(QColor(255, 255, 255, 230));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }
    else if (_pCurrentToolTip->_pTipStyle == PWToolTip::TextTip)
    {
        // 绘制带圆角的半透明背景
        QPainterPath path;
        path.addRoundedRect(rect(), 5, 5);
        painter.fillPath(path, QColor(255, 255, 224, 230));

        // 绘制边框
        painter.setPen(QPen(QColor(204, 204, 204, 200), 1));
        painter.drawPath(path);
    }
}

void PWToolTip::_updateContent(const QString& text, const QPoint& pos)
{
    QScreen* screen = QGuiApplication::screenAt(pos);
    if (!screen) return;

    // 步骤1：转换HTML为纯文本计算尺寸
    QTextDocument doc(text);
    QFontMetrics metrics(_pCurrentToolTip->_pContent->font());

    // 步骤2：计算合理尺寸
    QRect textRect = metrics.boundingRect(
        QRect(0, 0, MAX_TOOLTIP_WIDTH, 0),
        Qt::TextWordWrap | Qt::AlignLeft,
        doc.toPlainText()
    );
    // 步骤3：获取当前布局边距
    QMargins margins = layout()->contentsMargins();
    int horizontalMargin = margins.left() + margins.right();
    int verticalMargin = margins.top() + margins.bottom();

    int contentWidth = 0;
    int contentHeight = 0;
    if (_pCurrentToolTip->_pTipStyle == PWToolTip::TextTip) {
        contentWidth = textRect.width() + horizontalMargin * 2;
        contentHeight = textRect.height() + verticalMargin * 2;
    }
    else {
        contentWidth = textRect.width() + horizontalMargin;
        contentHeight = textRect.height() + verticalMargin;
    }
    contentWidth = std::min(contentWidth, MAX_TOOLTIP_WIDTH + horizontalMargin);

    setFixedSize(contentWidth + 2, contentHeight + 2);
    _pContent->setText(text);

    // 步骤5：定位
    move(pos);
}


