#include "PWTableView.h"
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QMimeData>
#include <QHBoxLayout>
#include <NXLineEdit.h>
#include <NXMenu.h>
#include <NXToggleSwitch.h>
#include <NXTheme.h>
#include "PWHeaderView.h"
#include "PWTableViewModel.h"
#include "PWTableViewIconDelegate.h"
#include "PWComboBox.h"

#include "PWLogger.h"
#include "PWUtils.h"
#include "PWToolTip.h"
namespace Scope::Utils {
    static QPointer<NXLineEdit> CreateRangeLineEdit(QWidget* parent) {
        static QRegularExpression regexPage("^[1-9]\\d*$");
        QPointer<NXLineEdit> lineEdit = new NXLineEdit(parent);
        lineEdit->setContentsPaddings(1, 0, 0, 0);
        lineEdit->setIsClearButtonEnabled(false);
        lineEdit->setFixedSize(50, 24);
        lineEdit->setBorderRadius(0);
        lineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lineEdit->setText(QString::number(1));
        lineEdit->setAlignment(Qt::AlignCenter);
        lineEdit->setValidator(new QRegularExpressionValidator(regexPage, lineEdit));
        return lineEdit;
    }
    static std::vector<std::array<int, 2>> MergeIntervals(std::vector<std::array<int, 2>>& intervals) {
        if (intervals.size() <= 1) return intervals;

        auto compare = [](const auto& v1, const auto& v2) { return v1.front() < v2.front(); };
        std::sort(intervals.begin(), intervals.end(), compare);

        std::vector<std::array<int, 2>> res;
        res.reserve(intervals.size());

        auto prev = intervals.begin(), next = intervals.begin() + 1;
        for (; next != intervals.end(); ++prev, ++next) {
            if ((*prev).back() < (*next).front()) {
                res.push_back(*prev);
            }
            else if ((*prev).back() > (*next).back()) {
                (*next).front() = (*prev).front();
                (*next).back() = (*prev).back();
            }
            else if ((*prev).back() >= (*next).front()) {
                (*next).front() = (*prev).front();
            }
        }
        res.push_back(*prev);

        return res;
    }
    
    static auto ValidateNumberRangeFunc(const QString& text, int totalPage) -> bool {// 验证数字范围是否有效S
        QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) return false;

        bool hasMultipleDash = trimmed.count('-') > 1;
        bool startsWithDash = trimmed.startsWith('-');
        bool endsWithDash = trimmed.endsWith('-');
        if (hasMultipleDash || startsWithDash || endsWithDash) return false;

        if (trimmed.contains('-')) {
            QStringList nums = trimmed.split('-');
            if (nums.size() != 2) return false;

            bool ok1, ok2;
            int start = nums[0].toInt(&ok1);
            int end = nums[1].toInt(&ok2);

            return ok1 && ok2 && start <= end && start <= totalPage && end <= totalPage;
        }
        else {
            bool ok;
            int num = trimmed.toInt(&ok);
            return ok && num <= totalPage;
        }
    };

    static auto ProcessPdfActionFormatFunc(auto& rowDataRange, const QStringList& parts, int totalPage) -> QString {
        std::vector<std::array<int, 2>> validParts;
        validParts.reserve(parts.size());

        for (const QString& part : parts) {
            if (!ValidateNumberRangeFunc(part, totalPage)) continue;

            QString trimmed = part.trimmed();
            if (trimmed.contains('-')) {
                QStringList nums = trimmed.split('-');
                int start = nums[0].toInt();
                int end = nums[1].toInt();
                validParts.push_back({ start, end });
            }
            else {
                int num = trimmed.toInt();
                validParts.push_back({ num, num });
            }
        }

        if (validParts.empty()) return QString();

        std::vector<std::array<int, 2>> intervals = Scope::Utils::MergeIntervals(validParts);
        QStringList mergedParts;
        mergedParts.reserve(intervals.size());

        for (const auto& interval : intervals) {
            if (interval[0] == interval[1]) {
                rowDataRange.append({.Start = interval[0],.End = interval[0] });
                mergedParts.append(QString::number(interval[0]));
            }
            else {
                rowDataRange.append({.Start = interval[0],.End = interval[1] });
                mergedParts.append(QString("%1-%2").arg(interval[0]).arg(interval[1]));
            }
        }

        return mergedParts.join(QChar(0x3001));
    };


    
    static auto ProcessPdfActionNoMergeFormatFunc(auto& rowDataRange, const QStringList& parts, int totalPage) -> QString {
        QSet<QString> validParts;
        validParts.reserve(parts.size());

        for (const QString& part : parts) {
            if (!ValidateNumberRangeFunc(part, totalPage)) continue;

            QString trimmed = part.trimmed();
            if (trimmed.contains('-')) {
                QStringList nums = trimmed.split('-');
                int start = nums[0].toInt();
                int end = nums[1].toInt();
                validParts << QString("%1-%2").arg(start).arg(end);
            }
            else {
                validParts << trimmed;
            }
        }

        if (validParts.empty()) return QString();
        QString result;
        result.reserve(validParts.size() * 2);
        for (auto it = validParts.begin(); it!= validParts.end(); ++it) {
            if ((*it).size() == 1) {
                rowDataRange.append({.Start = (*it).toInt(),.End = (*it).toInt() });
            }
            else {
                rowDataRange.append({ .Start = (*it)[0].digitValue(),.End = (*it)[2].digitValue() });
            }
            result.append(*it);
            result.append(QChar(0x3001));
        }
        result.chop(1);
        return result;
    };

}
QColor gIndicatorColor = NXThemeColor(nxTheme->getThemeMode(), PrimaryNormal);

using namespace WizConverter::Enums;
PWTableView::PWTableView(QWidget* parent)
    : NXTableView(parent)
{
    _pHeaderView = new PWHeaderView(Qt::Horizontal, this);
    _pModel = new PWTableViewModel(this);
    setFontSize(13);
    setMouseTracking(true);
    setAlternatingRowColors(true);
    setIconSize(TABLE_VIEW_CHECKICON_SIZE);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setModel(_pModel);
    setHorizontalHeader(_pHeaderView);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setFixedHeight(40);
    verticalHeader()->setMinimumSectionSize(48);
    verticalHeader()->setHidden(true);
    horizontalScrollBar()->setVisible(false);

    adjustHeaderColumnIconRect({ { 0, {2, -8, 10, 10} } });

    QObject::connect(this, &NXTableView::tableViewShow, this,  &PWTableView::_onTableViewShow, Qt::SingleShotConnection);
    QObject::connect(this, &PWTableView::checkIconClicked, _pModel,  &PWTableViewModel::onSelectSingleRow);
    QObject::connect(this, &PWTableView::swapRows, _pModel, &PWTableViewModel::onSwapRows);
    QObject::connect(_pHeaderView, &PWHeaderView::selectAllRows, _pModel, &PWTableViewModel::onSelectAllRows);
    QObject::connect(_pHeaderView, &PWHeaderView::removeSelectedRows, _pModel, &PWTableViewModel::onRemoveSelectedRows);
}

PWTableView::~PWTableView()
{
}

void PWTableView::setModuleType(WizConverter::Enums::ModuleType moduleType)
{
    _pModuleType = moduleType;
    if ((_pModuleType.MasterModuleType == MasterModule::Type::ImageAction && qvariant_cast<ImageAction::Type>(_pModuleType.SlaveModuleType) != ImageAction::Type::PDFToImage)
        || qvariant_cast<WordToPDF::Type>(_pModuleType.SlaveModuleType) == WordToPDF::Type::ImageToPDF) {
        std::call_once(_pOnceFlag, [this]() {
            NXToggleSwitch* toggleSwitchButton = new NXToggleSwitch(this);
            toggleSwitchButton->setCursor(Qt::PointingHandCursor);
            toggleSwitchButton->setToolTip("显示图片");
            toggleSwitchButton->setIsToggled(false);
            toggleSwitchButton->setVisible(true);
            toggleSwitchButton->move(300, 10);

            QObject::connect(toggleSwitchButton, &NXToggleSwitch::toggled, this,
                [toggleSwitchButton, this](bool checked) {
                _pModel->setProperty("IsGridViewMode", checked);
                if (_pModel->property("IsGridViewMode").toBool()) {
                    _pModel->resetRemoveIndexWidgits();
                    setIsHoverEffectsEnabled(false);
                    setIsSelectionEffectsEnabled(false);
                    setAlternatingRowColors(false);
                    setAcceptDrops(true);
                    setDropIndicatorShown(true);
                    setDragEnabled(true);
                    setDragDropMode(QAbstractItemView::InternalMove);
                    setSelectionBehavior(QAbstractItemView::SelectItems);
                    //setSelectionMode(QAbstractItemView::MultiSelection);
                    verticalHeader()->setDefaultSectionSize(145);
                    setHeaderTextList({ "","","","" });
                    _pModel->layoutChanged();

                    for (int i = 0; i < 4; ++i) {
                        setColumnWidth(i, 244);
                        setItemDelegateForColumn(i, _pIconDelegate);
                    }
                }
                else {
                    _pModel->resetRecoverIndexWidgits();
                    setIsHoverEffectsEnabled(true);
                    setIsSelectionEffectsEnabled(true);
                    setAlternatingRowColors(true);
                    setAcceptDrops(false);
                    setDropIndicatorShown(false);
                    setDragEnabled(false);
                    setDragDropMode(QAbstractItemView::NoDragDrop);
                    setSelectionBehavior(QAbstractItemView::SelectRows);
                    setSelectionMode(QAbstractItemView::SingleSelection);
                    verticalHeader()->setDefaultSectionSize(50);
                    verticalScrollBar()->setRange(0, _pModel->rowCount() * 50);
                    setHeaderTextList(_pModel->property("HeaderTextList").toStringList());
                    _pModel->layoutChanged();

                    int i = 0;
                    for ( ; i < _pColumnWidthList.size(); ++i) {
                        setColumnWidth(i, _pColumnWidthList[i]);
                        setItemDelegateForColumn(i, nullptr);
                    }
                    setItemDelegateForColumn(_pColumnWidthList.size() - 2, _pIconDelegate);
                    setItemDelegateForColumn(_pColumnWidthList.size() - 1, _pIconDelegate);
                    setSelection(visualRectForRow(currentIndex().row() * 4 + currentIndex().column()), QItemSelectionModel::SelectCurrent);
                }
                toggleSwitchButton->setToolTip(checked ? "切换到列表视图" : "切换到网格视图");
                static std::once_flag resetFlag;
                std::call_once(resetFlag, [this]() {
                    QObject::connect(_pModel, &QAbstractItemModel::rowsInserted, this, [this]() {
                        if(_pModel->property("IsGridViewMode").toBool())
                            _pModel->resetRemoveIndexWidgits();
                        this->_updateGridViewScrollBar();
                        });
                    QObject::connect(_pModel, &QAbstractItemModel::rowsRemoved, this, &PWTableView::_updateGridViewScrollBar);
                    });
                });
            });
    }
}

WizConverter::Enums::ModuleType PWTableView::getModuleType() const
{
    return _pModuleType;
}

void PWTableView::setHeaderTextList(const QStringList& headerTextList)
{
    _pModel->setHeaderTextList(headerTextList);
    update();
}

void PWTableView::setColumnWidthList(const QList<int>& columnWidthList)
{
    if(_pModel->columnCount() != columnWidthList.size())
    {
        //PW_ERROR("column count not match");
        return;
    }
    _pColumnWidthList = columnWidthList;
}

NXModelIndexWidget* PWTableView::createLineEditRangeWidget()
{
    if (_pModuleType.MasterModuleType == MasterModule::Type::PDFAction) {
        if (const auto& pdfType = qvariant_cast<PDFAction::Type>(_pModuleType.SlaveModuleType);
            pdfType == PDFAction::Type::PDFCompress || pdfType == PDFAction::Type::PDFCrypto)
        {
            return nullptr;
        }
        return createOneLineEditRangeWidget();
    }
    if (_pModuleType.MasterModuleType == MasterModule::Type::ImageAction)
    {
        if (const auto& imageType = qvariant_cast<ImageAction::Type>(_pModuleType.SlaveModuleType);
            !(imageType == ImageAction::Type::PDFToImage || imageType == ImageAction::Type::ImageToPDF
                || imageType == ImageAction::Type::ImageResize))
        {
            return nullptr;
        }
    }
    return createTwoLineEditRangeWidget();
}

NXModelIndexWidget* PWTableView::createTwoLineEditRangeWidget()
{
    QPointer<NXModelIndexWidget> modelIndexWidget = new NXModelIndexWidget(this);
    QPointer<QHBoxLayout> modelIndexLayout = new QHBoxLayout(modelIndexWidget);
    modelIndexLayout->setContentsMargins(0, 0, 0, 0);
    modelIndexLayout->setSpacing(0);
    modelIndexLayout->setAlignment(Qt::AlignCenter);

    QPointer<NXLineEdit> leftLineEdit = Scope::Utils::CreateRangeLineEdit(this);
    QPointer<NXLineEdit> rightLineEdit = Scope::Utils::CreateRangeLineEdit(this);

    QPointer<QLabel> label = new QLabel("—", this);
    label->setStyleSheet("QLabel{border:none;}");
    label->setFixedWidth(8);
    label->setAlignment(Qt::AlignCenter);

    modelIndexLayout->addStretch();
    modelIndexLayout->addWidget(leftLineEdit);
    modelIndexLayout->addSpacing(5);
    modelIndexLayout->addWidget(label);
    modelIndexLayout->addSpacing(5);
    modelIndexLayout->addWidget(rightLineEdit);
    modelIndexLayout->addStretch();

    // 实时验证（提供即时反馈）
    QObject::connect(leftLineEdit, &NXLineEdit::textChanged, [this, leftLineEdit, rightLineEdit](const QString& text) {
        const QModelIndex& index = this->indexAt(viewport()->mapToParent(leftLineEdit->parentWidget()->pos()));
        if (!index.isValid()) return;

        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        if (rowData.isRange()) {
            int leftValue = text.toInt();
            int rightValue = rightLineEdit->text().toInt();
            int maxValue = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn).Max;

            if (leftValue > maxValue || leftValue > rightValue) {
                leftLineEdit->setStyleSheet("border: 1px solid red;");
            }
            else {
                leftLineEdit->setStyleSheet("");
            }
        }
        });

    QObject::connect(rightLineEdit, &NXLineEdit::textChanged, [this, leftLineEdit, rightLineEdit](const QString& text) {
        const QModelIndex& index = this->indexAt(viewport()->mapToParent(rightLineEdit->parentWidget()->pos()));
        if (!index.isValid()) return;

        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        if (rowData.isRange()) {
            int rightValue = text.toInt();
            int leftValue = leftLineEdit->text().toInt();
            int maxValue = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn).Max;

            if (rightValue > maxValue || rightValue < leftValue) {
                rightLineEdit->setStyleSheet("border: 1px solid red;");
            }
            else {
                rightLineEdit->setStyleSheet("");
            }
        }
        });

    // 连接编辑完成信号
    QObject::connect(leftLineEdit, &NXLineEdit::editingFinished, [this, leftLineEdit, rightLineEdit]() {
        const QModelIndex& index = this->indexAt(viewport()->mapToParent(leftLineEdit->parentWidget()->pos()));
        if (!index.isValid()) return;

        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        int leftValue = leftLineEdit->text().toInt();
        int rightValue = rightLineEdit->text().toInt();

        if (rowData.isRange()) {
            // 获取Range的限制
            auto& range = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn);
            int maxValue = range.Max;
            int originalStart = range.RangeList.front().Start;
            if (maxValue <= 0 || originalStart == leftValue) return;

            if (leftValue > maxValue) leftValue = maxValue;
            if (leftValue > rightValue) leftValue = rightValue;

            // 更新显示
            if (leftLineEdit->text().toInt() != leftValue) {
                leftLineEdit->setText(QString::number(leftValue));
            }

            range.RangeList.front().Start = leftValue;
        }
        else if (rowData.isSize()) {
            auto& size = std::get<PWTableViewModel::ImageSizeData>(rowData.SecondColumn);
            int originalWidth = size.OriginalSize.width();
            if (leftValue == originalWidth) return;

            size.ResizedSize.setWidth(leftValue);
        }

        //// 通知模型数据已更改
        //QModelIndex dataIndex = _pModel->index(index.row(), 2); // 第二列数据
        //_pModel->dataChanged(dataIndex, dataIndex);
        });

    QObject::connect(rightLineEdit, &NXLineEdit::editingFinished, [this, leftLineEdit, rightLineEdit]() {
        const QModelIndex& index = this->indexAt(viewport()->mapToParent(rightLineEdit->parentWidget()->pos()));
        if (!index.isValid()) return;

        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        int leftValue = leftLineEdit->text().toInt();
        int rightValue = rightLineEdit->text().toInt();

        if (rowData.isRange()) {
            auto& range = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn);
            int maxValue = range.Max;
            int originalEnd = range.RangeList.front().End;
            if (maxValue <= 0 || originalEnd == rightValue) return;

            if (rightValue > maxValue) rightValue = maxValue;
            if (rightValue < leftValue) rightValue = leftValue;

            if (rightLineEdit->text().toInt() != rightValue) {
                rightLineEdit->setText(QString::number(rightValue));
            }
            range.RangeList.front().End = rightValue;
        }
        else if (rowData.isSize()) {
            auto& size = std::get<PWTableViewModel::ImageSizeData>(rowData.SecondColumn);
            int originalHeight = size.OriginalSize.height();
            if (rightValue == originalHeight) return;

            size.ResizedSize.setHeight(rightValue);
        }
        });

    // 当widget的index改变时更新显示
    QObject::connect(modelIndexWidget, &NXModelIndexWidget::indexChanged, [this, leftLineEdit, rightLineEdit](const QModelIndex& index) {
        if (!index.isValid()) return;

        const PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        if (rowData.isRange()) {
            const auto& range = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn);
            leftLineEdit->setText(QString::number(range.RangeList.front().Start));
            rightLineEdit->setText(QString::number(range.RangeList.front().End));
        }
        else if (rowData.isSize()) {
            const auto& size = std::get<PWTableViewModel::ImageSizeData>(rowData.SecondColumn);
            leftLineEdit->setText(QString::number(size.ResizedSize.width()));
            rightLineEdit->setText(QString::number(size.ResizedSize.height()));
        }
        });
    return modelIndexWidget;
}

NXModelIndexWidget* PWTableView::createOneLineEditRangeWidget()
{
    static QRegularExpression regexPattern("^["
        "0-9 "                               // 数字和空格
        ",，;；、.。!！?？"                  // 中英标点
        "‘’'\"<>《》〈〉【】『』「」"        // 引号和括号
        "|"                                  // 竖线
        "+\\-*/"                             // 运算符
        "\\\\"                               // 反斜线
        "]*$");
    static QRegularExpression splitPattern(R"([\s,，;；、.。!！?？‘’'""<>《》〈〉【】『』「」|+*/\\])");
    static QRegularExpression dashRegex("\\s*-+\\s*");

    QPointer<NXModelIndexWidget> modelIndexWidget = new NXModelIndexWidget(this);
    QPointer<QHBoxLayout> modelIndexLayout = new QHBoxLayout(modelIndexWidget);
    modelIndexLayout->setContentsMargins(0, 0, 0, 0);
    modelIndexLayout->setSpacing(0);
    modelIndexLayout->setAlignment(Qt::AlignCenter);

    QPointer<NXLineEdit> pageRangeLineEdit = new NXLineEdit(this);
    pageRangeLineEdit->setPlaceholderText("示例：1-3,4、5-6");
    pageRangeLineEdit->setIsClearButtonEnabled(true);
    pageRangeLineEdit->setFixedSize(145, 24);
    pageRangeLineEdit->setBorderRadius(0);
    pageRangeLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pageRangeLineEdit->setAlignment(Qt::AlignLeft);
    pageRangeLineEdit->setValidator(new QRegularExpressionValidator(regexPattern, pageRangeLineEdit));

    modelIndexLayout->addStretch();
    modelIndexLayout->addWidget(pageRangeLineEdit);
    modelIndexLayout->addStretch();

    QObject::connect(pageRangeLineEdit, &NXLineEdit::focusIn, modelIndexWidget, [pageRangeLineEdit](const QString& text) {
        if (text.isEmpty()) return;
        const QString& fullText = pageRangeLineEdit->property("FullRangeText").toString();
        pageRangeLineEdit->blockSignals(true);
        pageRangeLineEdit->setText(fullText);
        pageRangeLineEdit->blockSignals(false);
    });
    QObject::connect(pageRangeLineEdit, &NXLineEdit::editingFinished, modelIndexWidget, [this, pageRangeLineEdit]() {
        const QModelIndex& index = this->indexAt(viewport()->mapToParent(pageRangeLineEdit->parentWidget()->pos()));
        if (!index.isValid()) return;
        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        auto& range = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn);
        QString processedText = pageRangeLineEdit->text().trimmed();
        if (processedText.isEmpty()) {
            range.RangeList.clear();
            range.RangeList.emplace_back(1, 1);
            pageRangeLineEdit->setProperty("FullRangeText", 1);
            pageRangeLineEdit->setToolTip("");
            pageRangeLineEdit->blockSignals(true);
            pageRangeLineEdit->setText("1");
            pageRangeLineEdit->blockSignals(false);
            return;
        } 
        if (processedText == pageRangeLineEdit->property("FullRangeText").toString()) {
            QFontMetrics metrics(pageRangeLineEdit->font());
            pageRangeLineEdit->blockSignals(true);
            pageRangeLineEdit->setText(metrics.elidedText(processedText, Qt::ElideRight, 120));
            pageRangeLineEdit->blockSignals(false);
            return;
        }
        const int& totalPage = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn).Max;

        processedText.replace(dashRegex, "-");
        const QStringList& parts = processedText.split(splitPattern, Qt::SkipEmptyParts);
        const auto& pdfActionType = qvariant_cast<PDFAction::Type>(_pModuleType.SlaveModuleType);

        QString formatted;
        range.RangeList.clear();
        if (pdfActionType == PDFAction::Type::PDFSplit
            || pdfActionType == PDFAction::Type::PDFMerge
            || pdfActionType == PDFAction::Type::PDFPageRenderAsImage
            || pdfActionType == PDFAction::Type::PDFPageDelete
            || pdfActionType == PDFAction::Type::PDFWatermark
            || pdfActionType == PDFAction::Type::DocumentTranslate) {
            formatted = Scope::Utils::ProcessPdfActionFormatFunc(range.RangeList, parts, totalPage);
        }
        else {
            formatted = Scope::Utils::ProcessPdfActionNoMergeFormatFunc(range.RangeList, parts, totalPage);
        }

        if (formatted.isEmpty()) {
            range.RangeList.emplace_back(1, 1);
            pageRangeLineEdit->setProperty("FullRangeText", 1);
            pageRangeLineEdit->setToolTip("");
            pageRangeLineEdit->blockSignals(true);
            pageRangeLineEdit->setText("1");
            pageRangeLineEdit->blockSignals(false);
            return;
        }

        QFontMetrics metrics(pageRangeLineEdit->font());
        pageRangeLineEdit->setProperty("FullRangeText", formatted);
        pageRangeLineEdit->setToolTip(formatted);
        formatted = metrics.elidedText(formatted, Qt::ElideRight, 120);
        pageRangeLineEdit->blockSignals(true);
        pageRangeLineEdit->setText(formatted);
        pageRangeLineEdit->blockSignals(false);
    });
    QObject::connect(modelIndexWidget, &NXModelIndexWidget::indexChanged, this, [this, pageRangeLineEdit](const QModelIndex& index) {
        if (!index.isValid()) return;
        PWTableViewModel::RowData& rowData = _pModel->getRowData(index.row());
        auto& range = std::get<PWTableViewModel::RangeData>(rowData.SecondColumn);
        range.RangeList.front().End = 1;
        pageRangeLineEdit->blockSignals(true);
        pageRangeLineEdit->setText(QString::number(range.RangeList.front().Start));
        pageRangeLineEdit->blockSignals(false);
        pageRangeLineEdit->setProperty("FullRangeText", range.RangeList.front().Start);
    });

    return modelIndexWidget;
}

NXModelIndexWidget* PWTableView::createSwitchWidget()
{
    QPointer<NXModelIndexWidget> modelIndexWidget = new NXModelIndexWidget(this);
    QPointer<QHBoxLayout> modelIndexLayout = new QHBoxLayout(modelIndexWidget);
    modelIndexLayout->setContentsMargins(0, 0, 0, 0);
    modelIndexLayout->setSpacing(0);
    modelIndexLayout->setAlignment(Qt::AlignCenter);

    QPointer<PWComboBox> comboBox = new PWComboBox(this);
    comboBox->setCursor(Qt::PointingHandCursor);
    comboBox->setFixedSize(110, 26);
    comboBox->addItems({ "WORD","EXCEL","PPT","IMAGE","TXT","CAD","EPUB","HTML", "XML" });
    comboBox->setCurrentIndex(_pModuleType.SlaveModuleType.toInt());
    comboBox->setClickSwitchEnabled(false);
    modelIndexLayout->addWidget(comboBox);

    QObject::connect(comboBox, QOverload<int>::of(&PWComboBox::itemClicked), this, &PWTableView::switchClicked);
    return modelIndexWidget;
}

QRect PWTableView::visualRectForRow(int row) const
{
    QRect rowRect;
    for (int col = 0; col < model()->columnCount(); ++col) {
        if (!isColumnHidden(col)) {
            QModelIndex index = model()->index(row, col);
            QRect rect = visualRect(index);
            if (rect.isValid()) {
                rowRect = rowRect.isNull() ? rect : rowRect.united(rect);
            }
        }
    }
    return rowRect;
}

void PWTableView::paintEvent(QPaintEvent* event)
{
    NXTableView::paintEvent(event);
    const QModelIndex& targetIndex = property("PWTargetIndex").value<QModelIndex>();
    if (!targetIndex.isValid()) return;
    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = visualRectForRow(targetIndex.row());

    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(QPen(gIndicatorColor, 2));

    switch (property("PWDropIndicatorPosition").value<DropIndicatorPosition>()) {
    case QAbstractItemView::AboveItem: {
        int startY = option.rect.top() + 1;
        int startX = option.rect.left() + 6;
        painter.drawEllipse(QPoint(startX, startY), 4, 4);
        painter.drawLine(QPoint(startX, startY), QPoint(option.rect.width() + 6, startY));
        break;
    }
    case QAbstractItemView::BelowItem: {
        int startY = option.rect.bottom();
        int startX = option.rect.left() + 6;
        painter.drawEllipse(QPoint(startX, startY), 4, 4);
        painter.drawLine(QPoint(startX, startY), QPoint(option.rect.width() + 6, startY));
        break;
    }
    case QAbstractItemView::OnItem: {
        painter.drawRect(option.rect.adjusted(1, 1, -1, -1));
        break;
    }
    default: break;
    }
}

void PWTableView::mouseReleaseEvent(QMouseEvent* event)
{
    if (selectionBehavior() == QAbstractItemView::SelectRows && event->button() == Qt::LeftButton && !model()->property("IsGridViewMode").toBool())
    {
        const QModelIndex& currentIndex = indexAt(event->pos());
        if (currentIndex.column() == 0)
        {
            QRect iconRect = WizConverter::Utils::GetAlignLeft(visualRect(currentIndex), TABLE_VIEW_CHECKICON_SIZE);
            iconRect.adjust(TABLE_VIEW_CHECKICON_ADJUST);
            if (iconRect.contains(event->pos())) {
                Q_EMIT checkIconClicked(currentIndex);
            }
        }
        else if (currentIndex.column() == 1) {
            // 22 x 22 是FileIcon的大小, 118 是需要裁剪的宽度
            QRect textRect = WizConverter::Utils::GetAlignLeft(visualRect(currentIndex), QSize{ columnWidth(currentIndex.column()) - 118, 22 });
            if (textRect.contains(event->pos())) {
                Q_EMIT checkIconClicked(currentIndex);
            }
        }
    }

    NXTableView::mouseReleaseEvent(event);
}

void PWTableView::mouseMoveEvent(QMouseEvent* event)
{
    if (!model()->property("IsGridViewMode").toBool()) {
        unsetCursor();
        const QModelIndex& currentIndex = indexAt(event->pos());
        if (currentIndex.column() == 0)
        {
            QRect iconRect = WizConverter::Utils::GetAlignLeft(visualRect(currentIndex), TABLE_VIEW_CHECKICON_SIZE);
            iconRect.adjust(TABLE_VIEW_CHECKICON_ADJUST);
            if (iconRect.contains(event->pos())) {
                setCursor(Qt::PointingHandCursor);
            }
            PWToolTip::hideToolTip();
        }
        else if (currentIndex.column() == 1) {
            QRect textRect = WizConverter::Utils::GetAlignLeft(visualRect(currentIndex), QSize{ columnWidth(currentIndex.column()) - 118, 22 });
            if (textRect.contains(event->pos())) {
                setCursor(Qt::PointingHandCursor);
                const QString& fileName = qobject_cast<PWTableViewModel*>(model())->getRowData(currentIndex.row()).FileName;
                PWToolTip::showToolTip(fileName, viewport()->mapToGlobal(textRect.bottomLeft() + QPoint(0, 1)));
            }
            else PWToolTip::hideToolTip();
        }
    }
    
    NXTableView::mouseMoveEvent(event);
}

void PWTableView::leaveEvent(QEvent* event)
{
    PWToolTip::hideToolTip();
    unsetCursor();
    NXTableView::leaveEvent(event);
}

void PWTableView::contextMenuEvent(QContextMenuEvent* event)
{
    NXMenu* menu = new NXMenu(this);
    menu->setMenuItemHeight(27);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction* action = nullptr;

    action = menu->addNXIconAction(NXIconType::ArrowUpAZ, tr("按文件名降序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileNameActionTriggered(true);  // 降序
        });

    action = menu->addNXIconAction(NXIconType::ArrowDownAZ, tr("按文件名升序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileNameActionTriggered(false); // 升序
        });
    menu->addSeparator();

    action = menu->addNXIconAction(NXIconType::FileArrowUp, tr("按文件大小降序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileSizeActionTriggered(true);
        });

    action = menu->addNXIconAction(NXIconType::FileArrowDown, tr("按文件大小升序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileSizeActionTriggered(false);
        });
    menu->addSeparator();

    action = menu->addNXIconAction(NXIconType::ClockTwelve, tr("按文件创建时间降序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileBirthTimeActionTriggered(true);
        });

    action = menu->addNXIconAction(NXIconType::ClockSixThirty, tr("按文件创建时间升序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByFileBirthTimeActionTriggered(false);
        });
    menu->addSeparator();

    action = menu->addNXIconAction(NXIconType::ArrowUp19, tr("按总页数或图片大小降序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByRangeOrSizeActionTriggered(true);
        });

    action = menu->addNXIconAction(NXIconType::ArrowDown19, tr("按总页数或图片大小升序"));
    QObject::connect(action, &QAction::triggered, _pModel, [this]() {
        _pModel->onSortByRangeOrSizeActionTriggered(false);
        });

    menu->popup(event->globalPos());
}

void PWTableView::resizeEvent(QResizeEvent* event)
{
    NXTableView::resizeEvent(event);
}

void PWTableView::dragEnterEvent(QDragEnterEvent* event)
{
    event->setAccepted(event->mimeData()->hasFormat("application/x-pwtableviewmodel-row"));
}

void PWTableView::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event->mimeData()->hasFormat("application/x-pwtableviewmodel-row")) {
        event->ignore();
        return;
    }
    QByteArray encodedData = event->mimeData()->data("application/x-pwtableviewmodel-row");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int draggedRow, draggedColumn;
    stream >> draggedRow >> draggedColumn;
    if (_pModel->property("IsGridViewMode").toBool()) {
        event->accept();
        return;
    }
    const QModelIndex& targetIndex = indexAt(event->position().toPoint());
    const QModelIndex& draggedIndex = _pModel->index(draggedRow, draggedColumn);
    DropIndicatorPosition dropindicationPos = dropIndicatorPositionOverride();
    setProperty("PWTargetIndex", targetIndex);
    setProperty("PWDraggedIndex", draggedIndex);
    setProperty("PWDropIndicatorPosition", dropindicationPos);
    const QModelIndex& draggedPreviousIndex = _pModel->index(draggedIndex.row() - 1, draggedColumn);
    const QModelIndex& draggedNextIndex = _pModel->index(draggedIndex.row() + 1, draggedColumn);

    if (targetIndex.row() == draggedIndex.row() || 
        (draggedNextIndex.isValid() && draggedNextIndex.row() == targetIndex.row() && dropindicationPos == QAbstractItemView::AboveItem) ||
        (draggedPreviousIndex.isValid() && draggedPreviousIndex.row() == targetIndex.row() && dropindicationPos == QAbstractItemView::BelowItem))
    {
        gIndicatorColor = NXThemeColor(nxTheme->getThemeMode(), BasicText);
    }
    else {
        if (dropindicationPos == QAbstractItemView::OnItem) {
            gIndicatorColor = nxTheme->getThemeMode() == NXThemeType::ThemeMode::Light
                ? QColor(0x7D, 0x52, 0x84)
                : QColor(0x66, 0x3D, 0x74);
        }
        else {
            gIndicatorColor = NXThemeColor(nxTheme->getThemeMode(), PrimaryNormal);
        }
    }

    viewport()->update();
    event->accept();
}

void PWTableView::dropEvent(QDropEvent* event)
{
    if (_pModel->property("IsGridViewMode").toBool()) {
        QByteArray encodedData = event->mimeData()->data("application/x-pwtableviewmodel-row");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        int draggedRow, draggedColumn;
        stream >> draggedRow >> draggedColumn;
        const QModelIndex& targetIndex = indexAt(event->position().toPoint());
        Q_EMIT swapRows(draggedRow * 4 + draggedColumn, targetIndex.row() * 4 + targetIndex.column());
        event->acceptProposedAction();
        return;
    }
    QModelIndex targetIndex = property("PWTargetIndex").value<QModelIndex>();
    QModelIndex draggedIndex = property("PWDraggedIndex").value<QModelIndex>();
    QAbstractItemView::DropIndicatorPosition dropindicationPos = property("PWDropIndicatorPosition").value<DropIndicatorPosition>();
    const QModelIndex& draggedPreviousIndex = _pModel->index(draggedIndex.row() - 1, 0);
    const QModelIndex& draggedNextIndex = _pModel->index(draggedIndex.row() + 1, 0);
    setProperty("PWTargetIndex", QModelIndex());
    setProperty("PWDraggedIndex", QModelIndex());
    if (targetIndex.row() == draggedIndex.row() ||
        (draggedNextIndex.isValid() && draggedNextIndex.row() == targetIndex.row() && dropindicationPos == QAbstractItemView::AboveItem) ||
        (draggedPreviousIndex.isValid() && draggedPreviousIndex.row() == targetIndex.row() && dropindicationPos == QAbstractItemView::BelowItem))
    {
        event->ignore();
    }
    else {
        if (dropindicationPos == QAbstractItemView::OnItem) {
            Q_EMIT swapRows(draggedIndex.row(), targetIndex.row());
            event->acceptProposedAction();
        }
        else {
            if (!_pModel->canDropMimeData(event->mimeData(), Qt::MoveAction, targetIndex.row(), 0, rootIndex())) {
                event->ignore(); return;
            }
            _pModel->dropMimeData(event->mimeData(), Qt::MoveAction, targetIndex.row(), 0, rootIndex());
            event->acceptProposedAction();
        }
    }
    setProperty("PWDropIndicatorPosition", QAbstractItemView::OnViewport);
}

QAbstractItemView::DropIndicatorPosition PWTableView::dropIndicatorPositionOverride() const
{
    const QModelIndex& targetIndex = property("PWTargetIndex").value<QModelIndex>();
    if (!targetIndex.isValid()) {
        return QAbstractItemView::OnViewport;
    }

    const QRect& itemRect = visualRect(targetIndex);
    const QPoint& cursorPos = mapFromGlobal(QCursor::pos() - QPoint(0, 45));
    if (cursorPos.y() - 4 < itemRect.top()) {
        return QAbstractItemView::AboveItem;
    }
    else if (cursorPos.y() + 12 > itemRect.bottom()) {
        return QAbstractItemView::BelowItem;
    }
    else {
        return QAbstractItemView::OnItem;
    }
}

void PWTableView::_onTableViewShow()
{
    _pModel->setProperty("HeaderTextList", _pModel->getHeaderTextList());
    for (int i = 0; i < _pColumnWidthList.size(); ++i)
    {
        setColumnWidth(i, _pColumnWidthList[i]);
    }
    _pIconDelegate = new PWTableViewIconDelegate(this);
    setItemDelegateForColumn(_pColumnWidthList.size() - 2, _pIconDelegate);
    setItemDelegateForColumn(_pColumnWidthList.size() - 1, _pIconDelegate);
    QObject::connect(_pIconDelegate, &PWTableViewIconDelegate::iconClicked, _pModel, &PWTableViewModel::onDelegateIconClicked);
}

void PWTableView::_updateGridViewScrollBar()
{
    int totalRows = std::ceil(_pModel->rowCount() / 4.0);
    if (totalRows <= 2) {
        verticalScrollBar()->setMaximum(145);
    }
    else {
        int scrollableRows = totalRows - 2;
        verticalScrollBar()->setMaximum(scrollableRows * 145);
    }
}
