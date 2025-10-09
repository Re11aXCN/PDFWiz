#include "PWTableViewIconDelegate.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHeaderView>
#include <QDesktopServices>
#include <QImageReader>
#include <QFileInfo>
#include <NXTheme.h>
#include <absl/strings/str_cat.h>

#include "PWUtils.h"
#include "PWTableView.h"
#include "PWToolTip.h"

constexpr std::size_t buffer_size = std::numeric_limits<int>::digits10 + 2;

PWTableViewIconDelegate::PWTableViewIconDelegate(QObject* parent)
    : QStyledItemDelegate{ parent }
    , _pCircleXImage(":/Resource/Image/Button/ImageAction_ITEM_CircleX.png")
    , _pDoubleClickHintImage(":/Resource/Image/Button/ImageAction_ITEM_DoubleClickHint.png")
{

}

PWTableViewIconDelegate::~PWTableViewIconDelegate()
{
}

void PWTableViewIconDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);

    if (option.state.testFlag(QStyle::State_HasFocus))
    {
        viewOption.state &= ~QStyle::State_HasFocus;
    }
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (index.data(Qt::UserRole + 1).toBool()/*property("IsGridViewMode").toBool()*/) {
        _paintGridViewMode(painter, viewOption, index);
    }
    else {
        _paintListViewMode(painter, viewOption, index);
    }

    QStyledItemDelegate::paint(painter, viewOption, index);
}

QSize PWTableViewIconDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

bool PWTableViewIconDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(!index.isValid()) return false;
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
    {
        viewOption.state &= ~QStyle::State_HasFocus;
    }

    const PWTableView* widget = qobject_cast<const PWTableView*>(viewOption.widget);

    if (index.data(Qt::UserRole + 1).toBool()/*property("IsGridViewMode").toBool()*/) {
        return _handleGridViewEditorEvent(event, viewOption, index, widget);
    }
    else {
        return _handleListViewEditorEvent(event, viewOption, index, widget);
    }
}

bool PWTableViewIconDelegate::eventFilter(QObject* obj, QEvent* event)
{
    return QStyledItemDelegate::eventFilter(obj, event);
}

// ==================== 私有方法实现 ====================

void PWTableViewIconDelegate::_paintGridViewMode(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFont font("NXAwesome");
    font.setPixelSize(13);
    painter->setFont(font);

    // 计算在ListView中的实际数据行索引
    // GridView中：行索引 * 4 + 列索引 = ListView中的行索引
    int listViewRowIndex = index.row() * 4 + index.column();
    // 获取模型
    const QAbstractItemModel* model = index.model();
    if (!model || listViewRowIndex >= model->rowCount()) {
        return; // 超出数据范围，不绘制
    }

    // 创建对应的ListView行索引（列索引为0，因为文件名数据在UserRole中）
    QModelIndex listViewIndex = model->index(listViewRowIndex, 0);
    const QString& fileName = listViewIndex.data(Qt::UserRole).toString(); // RowData::FileNam

    if (QImage image(fileName); !image.isNull()) {
        painter->setPen(!option.state.testFlag(QStyle::State_Enabled) ?
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicTextDisable) :
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicText));

        QRect imageRect(option.rect.x() + 25,
            option.rect.y() + 20,
            option.rect.width() - 50,
            100);
        QRect targetRect(imageRect.left() + 8,
            imageRect.top() + 1,
            imageRect.width() - 16,
            imageRect.height() - 1);
        painter->drawImage(targetRect, image);

        QRect textRect(option.rect.x() + 35,
            option.rect.y() + 120,
            option.rect.width() - 70,
            25);

        QFontMetrics metrics(painter->font());
        painter->drawText(textRect, Qt::AlignCenter, metrics.elidedText(fileName, Qt::ElideMiddle, textRect.width()));

        QRect backgroundRect(option.rect.x() + 15,
            option.rect.y() + 10,
            option.rect.width() - 30, // 左右各15
            135);

        if (option.state & QStyle::State_MouseOver/*option.rect.contains(localMousePos)*/) {
            // 计算圆形按钮的中心位置
            QRect buttonRect(imageRect.right() - _pCircleXImage.width() / 2,
                imageRect.top() - _pCircleXImage.height() / 2,
                _pCircleXImage.width(),
                _pCircleXImage.height());
            painter->drawImage(buttonRect, _pCircleXImage);

            painter->setPen(Qt::NoPen);  // 去除边框
            painter->setBrush(NXThemeColor(NXThemeType::ThemeMode::Light, BasicHoverAlpha));
            painter->drawRect(backgroundRect);

            painter->drawImage(WizConverter::Utils::GetAlignCenter(targetRect, _pDoubleClickHintImage.size()), _pDoubleClickHintImage);
        }
    }
}

void PWTableViewIconDelegate::_paintListViewMode(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static QChar icons[] = {
        QChar((unsigned short)NXIconType::Play),
        QChar((unsigned short)NXIconType::FileLines),
        QChar((unsigned short)NXIconType::FolderOpen),
        QChar((unsigned short)NXIconType::TrashCan)
    };

    QFont font("NXAwesome");
    QPoint localMousePos = _getAdjustedMousePosition(option);
    const PWTableView* widget = qobject_cast<const PWTableView*>(option.widget);

    int headerColumnCount = widget->horizontalHeader()->count();
    bool isSecondLastColumn = (index.column() == (headerColumnCount - 2));
    int iconCount = isSecondLastColumn ? 3 : 1;

    QList<QRect> cellRects = _getIconCellRects(option, widget, index);

    // 计算icon大小必须先设置NXAwesome字体,否则 DirectWrite: CreateFontFaceFromHDC() failed
    std::call_once(_pOnceFlag, [&]() {
        painter->setFont(font);
        _pIconSize = QSize(painter->fontMetrics().horizontalAdvance(icons[0]) + 10, painter->fontMetrics().height() + 8);
        });

    painter->save();
    for (int i = 0; i < iconCount; ++i) {
        QStyleOptionToolButton buttonOption;
        buttonOption.rect = WizConverter::Utils::GetAlignCenter(cellRects[i], _pIconSize);
        buttonOption.state = QStyle::State_Enabled;

        /*if (option.state & QStyle::State_Sunken) {
            buttonOption.state |= QStyle::State_Sunken;
        }*/
        if (buttonOption.rect.contains(localMousePos)) {
            buttonOption.state |= QStyle::State_MouseOver;
            painter->fillRect(buttonOption.rect, QColor(163, 199, 216));
            font.setPixelSize(17); // 悬停时放大
        }
        else {
            font.setPixelSize(16); // 正常大小
        }
        painter->setFont(font);

        painter->setPen(!option.state.testFlag(QStyle::State_Enabled) ?
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicTextDisable) :
            NXThemeColor(NXThemeType::ThemeMode::Light, BasicText));
        painter->drawText(buttonOption.rect, Qt::AlignCenter, icons[isSecondLastColumn ? i : 3]);
    }
    painter->restore();
}

bool PWTableViewIconDelegate::_handleGridViewEditorEvent(QEvent* event, const QStyleOptionViewItem& option, const QModelIndex& index, const PWTableView* widget)
{
    int listViewRowIndex = index.row() * 4 + index.column();
    const QAbstractItemModel* model = index.model();
    if (!model || listViewRowIndex >= model->rowCount()) return false;

    QPoint localMousePos = _getAdjustedMousePosition(option);
    QRect imageRect(option.rect.x() + 25,
        option.rect.y() + 20,
        option.rect.width() - 50,
        100);
    QRect targetRect(imageRect.left() + 8,
        imageRect.top() + 1,
        imageRect.width() - 16,
        imageRect.height() - 1);
    QRect buttonRect(imageRect.right() - _pCircleXImage.width() / 2,
        imageRect.top() - _pCircleXImage.height() / 2,
        _pCircleXImage.width(),
        _pCircleXImage.height());
    auto* tableView = const_cast<PWTableView*>(widget);
    switch (event->type()) {
    case QEvent::MouseButtonRelease: {
        if (buttonRect.contains(localMousePos)) {
            Q_EMIT iconClicked(IconRole::RemoveFile, model->index(listViewRowIndex, 0));
            widget->viewport()->unsetCursor();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonDblClick: {
        if (WizConverter::Utils::GetAlignCenter(targetRect, _pDoubleClickHintImage.size()).contains(localMousePos)) {
            QDesktopServices::openUrl(model->index(listViewRowIndex, 0).data(Qt::UserRole).toString());
            widget->viewport()->setCursor(Qt::PointingHandCursor);
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        if (bool isMouseOver = WizConverter::Utils::GetAlignCenter(targetRect, _pDoubleClickHintImage.size()).contains(localMousePos) || buttonRect.contains(localMousePos)) {
            widget->viewport()->setCursor(Qt::PointingHandCursor);
            if(isMouseOver) tableView->setToolTip(_formatFileInfo(model->index(listViewRowIndex, 0).data(Qt::UserRole).toString()));
        }
        else {
            tableView->setToolTip("");
            widget->viewport()->unsetCursor();
        }
        break;
    }
    case QEvent::Leave: {
        tableView->setToolTip("");
        widget->viewport()->unsetCursor();
        break;
    }
    }

    return false;
}

bool PWTableViewIconDelegate::_handleListViewEditorEvent(QEvent* event, const QStyleOptionViewItem& option, const QModelIndex& index, const PWTableView* widget)
{
    QPoint localMousePos = _getAdjustedMousePosition(option);

    int headerColumnCount = widget->horizontalHeader()->count();
    bool isSecondLastColumn = (index.column() == (headerColumnCount - 2));
    int iconCount = isSecondLastColumn ? 3 : 1;

    QList<QRect> cellRects = _getIconCellRects(option, widget, index);

    for (int i = 0; i < iconCount; ++i) {
        QStyleOptionToolButton buttonOption;
        buttonOption.rect = WizConverter::Utils::GetAlignCenter(cellRects[i], _pIconSize);
        buttonOption.state = QStyle::State_Enabled;

        if (buttonOption.rect.contains(localMousePos)) {
            if (event->type() == QEvent::MouseButtonRelease) {
                Q_EMIT iconClicked(isSecondLastColumn ? static_cast<IconRole>(i) : IconRole::RemoveFile, index);
                widget->viewport()->setCursor(Qt::PointingHandCursor);
                return true;
            }
            else if (event->type() == QEvent::MouseMove) {
                widget->viewport()->setCursor(Qt::PointingHandCursor);
                return true;
            }
        }
        else {
            widget->viewport()->unsetCursor();
        }
    }

    return false;
}

QPoint PWTableViewIconDelegate::_getAdjustedMousePosition(const QStyleOptionViewItem& option) const
{
    QPoint localMousePos = option.widget->mapFromGlobal(QCursor::pos());
    localMousePos.setY(localMousePos.y() - 45);     // 修正鼠标位置 1.5 倍表头的height
    return localMousePos;
}

QList<QRect> PWTableViewIconDelegate::_getIconCellRects(const QStyleOptionViewItem& option, const PWTableView* widget, const QModelIndex& index) const
{
    int headerColumnCount = widget->horizontalHeader()->count();
    bool isSecondLastColumn = (index.column() == (headerColumnCount - 2));
    int iconCount = isSecondLastColumn ? 3 : 1;

    return WizConverter::Utils::SplitRectHorizontally(
        isSecondLastColumn ? option.rect.adjusted(10, 0, -10, 0) : option.rect,
        iconCount
    );
}

QString PWTableViewIconDelegate::_formatFileInfo(const QString& fileName) const
{
    QImageReader imageReader(fileName);
    if (!imageReader.canRead()) return fileName + " (文件不存在或无法读取)";
    std::array<char, buffer_size> buffer;
    QFileInfo fileInfo(fileName);
    const QSize& imageSize = imageReader.size();
    auto result1 = std::to_chars(buffer.data(), buffer.data() + buffer.size(), imageSize.width(), 10);
    std::string str1(buffer.data(), result1.ptr);
    auto result2 = std::to_chars(result1.ptr, buffer.data() + buffer.size(), imageSize.height(), 10);
    std::string str2(result1.ptr, result2.ptr);
    qint64 fileSize = fileInfo.size();
    QString sizeStr;
    if (fileSize < 1024) {
        sizeStr = QString::number(fileInfo.size(), 'f', 2) + " 字节";
    }
    else if (fileSize < 1024 * 1024) {
        sizeStr = QString::number(fileSize / 1024.0, 'f', 2) + " KB";
    }
    else {
        sizeStr = QString::number(fileSize / 1024.0 / 1024.0, 'f', 2) + " MB";
    }
    return QString::fromStdString(absl::StrCat("<div style='font-family: Arial, sans-serif;'>",
        "<p><strong>文件：</strong>",
        fileName.toStdString(),
        "</p><p><strong>图片尺寸：</strong>",
        str1, " x ", str2, " 像素</p><p><strong>文件大小：</strong>",
        sizeStr.toStdString(), "</p><p><strong>文件创建时间：</strong>",
        fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss").toStdString(), "</p></div>"));
}
