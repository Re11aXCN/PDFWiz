#include "PWTableView.h"
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <NXLineEdit.h>

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
    setModel(_pModel);
    setHorizontalHeader(_pHeaderView);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setFixedHeight(40);
    verticalHeader()->setMinimumSectionSize(48);
    verticalHeader()->setHidden(true);
    horizontalScrollBar()->setVisible(false);

    adjustHeaderColumnIconRect({ { 0, {2, -8, 10, 10} } });

    QObject::connect(this, &NXTableView::tableViewShow, this,  &PWTableView::_onTableViewShow);
    QObject::connect(this, &PWTableView::checkIconClicked, _pModel,  &PWTableViewModel::onSelectSingleRow);
    QObject::connect(_pHeaderView, &PWHeaderView::selectAllRows, _pModel, &PWTableViewModel::onSelectAllRows);
    QObject::connect(_pHeaderView, &PWHeaderView::removeSelectedRows, _pModel, &PWTableViewModel::onRemoveSelectedRows);

}

PWTableView::~PWTableView()
{
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
    if (selectionBehavior() == QAbstractItemView::SelectRows && event->button() == Qt::LeftButton)
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
            const QString& fileName = qobject_cast<PWTableViewModel*>(model())->getCellData(currentIndex.row(), 1).toString();
            PWToolTip::showToolTip(fileName, viewport()->mapToGlobal(textRect.bottomLeft() + QPoint(0, 1)));
        }
        else PWToolTip::hideToolTip();
    }
    NXTableView::mouseMoveEvent(event);
}

void PWTableView::leaveEvent(QEvent* event)
{
    PWToolTip::hideToolTip();
    unsetCursor();
    NXTableView::leaveEvent(event);
}

void PWTableView::_onTableViewShow()
{
    for (int i = 0; i < _pColumnWidthList.size(); ++i)
    {
        setColumnWidth(i, _pColumnWidthList[i]);
    }
    PWTableViewIconDelegate* iconDelegate = new PWTableViewIconDelegate(this);
    setItemDelegateForColumn(_pColumnWidthList.size() - 2, iconDelegate);
    setItemDelegateForColumn(_pColumnWidthList.size() - 1, iconDelegate);
    QObject::connect(iconDelegate, &PWTableViewIconDelegate::iconClicked, _pModel, &PWTableViewModel::onDelegateIconClicked);
}

