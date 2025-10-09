#include "PWTableView.h"
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <NXLineEdit.h>
#include <NXMenu.h>
#include <NXToggleSwitch.h>

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
}
using namespace WizConverter::Enums;
PWTableView::PWTableView(QWidget* parent)
    : NXTableView(parent)
    , _pGridRowsPerPage{ 2 }
    , _pItemsPerRow{ 4 }
{
    _pHeaderView = new PWHeaderView(Qt::Horizontal, this);
    _pModel = new PWTableViewModel(this);
    setFontSize(13);
    setMouseTracking(true);
    setAlternatingRowColors(true);
    setIconSize(TABLE_VIEW_CHECKICON_SIZE);
    setSelectionBehavior(QAbstractItemView::SelectRows);
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
                Q_EMIT hideModelIndexWidget(checked);
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
                    setSelectionMode(QAbstractItemView::MultiSelection);
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
                    
                }
                toggleSwitchButton->setToolTip(checked ? "切换到列表视图" : "切换到网格视图");
                static std::once_flag hideModelIndexWidgetFlag;
                std::call_once(hideModelIndexWidgetFlag, [this]() {
                    QObject::connect(_pModel, &QAbstractItemModel::rowsInserted, this, [this]() {
                        if(_pModel->property("IsGridViewMode").toBool())
                            _pModel->resetRemoveIndexWidgits();
                        this->_updateGridViewScrollBar();
                        });
                    QObject::connect(_pModel, &QAbstractItemModel::rowsRemoved, this, &PWTableView::_updateGridViewScrollBar);
                    //QObject::connect(_pModel, &QAbstractItemModel::modelReset, this, &PWTableView::_updateGridViewScrollBar);
                    //QObject::connect(_pModel, &QAbstractItemModel::layoutChanged, this, &PWTableView::_updateGridViewScrollBar);
                    /*QObject::connect(_pModel, &QAbstractItemModel::rowsInserted, [this]() {
                        Q_EMIT hideModelIndexWidget(true);
                        });*/
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

NXModelIndexWidget* PWTableView::createRangeWidget()
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
            leftLineEdit->setText(QString::number(size.OriginalSize.width()));
            rightLineEdit->setText(QString::number(size.OriginalSize.height()));
        }
        });
    
    /*QObject::connect(this, &PWTableView::hideModelIndexWidget, modelIndexWidget, [modelIndexWidget](bool hide) {
        for (const auto& child : modelIndexWidget->children()) {
            if (auto* widget = qobject_cast<QWidget*>(child)) {
                widget->setVisible(!hide);
            }
        }
        modelIndexWidget->setVisible(!hide);
        });*/
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

void PWTableView::paintEvent(QPaintEvent* event)
{
    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    NXTableView::paintEvent(event);
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
    /*
    if (_pModel->property("IsGridViewMode").toBool()) {
        // 重新计算每页显示行数并更新滚动条
        _pGridRowsPerPage = _calculateGridRowsPerPage();
        _updateGridViewScrollBar();
    }*/
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
    if (!_pModel->property("IsGridViewMode").toBool()) {
        // 非网格模式，恢复默认滚动条逻辑
        //verticalScrollBar()->setRange(0, 1); // 让QTableView自动管理
        //verticalScrollBar()->setRange(0, 10);

        return;
    }

    int totalRows = _calculateGridTotalRows();
    if (totalRows <= _pGridRowsPerPage) {
        verticalScrollBar()->setMaximum(145);
    }
    else {
        int scrollableRows = totalRows - _pGridRowsPerPage;
        verticalScrollBar()->setMaximum(scrollableRows * 145);
    }
}

int PWTableView::_calculateGridRowsPerPage() const
{
    if (!_pModel->property("IsGridViewMode").toBool()) {
        return 0;
    }

    int viewportHeight = viewport()->height();
    int rowHeight = verticalHeader()->defaultSectionSize();

    if (rowHeight <= 0) {
        return _pGridRowsPerPage; // 使用默认值
    }

    return viewportHeight / rowHeight;
}

int PWTableView::_calculateGridTotalRows() const
{
    if (!_pModel->property("IsGridViewMode").toBool()) {
        return _pModel->rowCount();
    }

    int totalItems = _pModel->rowCount();
    // 计算总行数：ceil(totalItems / itemsPerRow)
    return (totalItems + _pItemsPerRow - 1) / _pItemsPerRow;
}

