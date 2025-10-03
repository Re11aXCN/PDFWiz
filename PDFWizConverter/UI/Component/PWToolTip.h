#pragma once
#include <QWidget>
class QLabel;
class PWToolTip : public QWidget {
    friend std::default_delete<PWToolTip>;
    Q_OBJECT
public:
    enum TipStyle
    {
        TextTip,
        WidgetTip
    };

    static void showToolTip(const QString& text, const QPoint& pos, TipStyle style = TextTip);
    static void hideToolTip();
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    explicit PWToolTip(QWidget* parent = nullptr);
    ~PWToolTip() = default;
    void _updateContent(const QString& text, const QPoint& pos);
    TipStyle _pTipStyle{ TextTip };
    QLabel* _pContent{ nullptr };
    inline static std::unique_ptr<PWToolTip> _pCurrentToolTip{ nullptr };
    inline static std::once_flag _pFlag;
};