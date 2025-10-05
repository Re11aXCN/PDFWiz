#include "PWCentralWidget.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QResizeEvent>
#include <QFileInfo>
#include <QTableWidgetItem>

#include "Component/PWTableMaskWidget.h"
#include "Component/PWTableView.h"
#include "Component/PWTableViewModel.h"
using namespace WizConverter::Enums;
namespace Scope::Utils {
    struct HeaderViewMetadata {
        QStringList TextList;
        QList<int> ColumnWidthList;
    };
    static HeaderViewMetadata GetHeaderViewMetadata(MasterModule::Type mType, const QVariant& sType) {
        switch (mType) {
        case MasterModule::Type::PDFToWord: {
            return { {"", "文件名", "总页数", "转换页面范围", "输出格式", "状态", "操作", "删除"},
                { 40, 300, 60, 145, 120, 140, 115, 57 } };
        }
        case MasterModule::Type::WordToPDF: {
            return qvariant_cast<WordToPDF::Type>(sType) == WordToPDF::Type::ImageToPDF
                ? HeaderViewMetadata{ {"", "文件名", "图片尺寸", "自定义尺寸", "状态", "操作", "删除"},
                    { 40, 310, 120, 165, 140, 135, 67 } }
                : HeaderViewMetadata{ {"", "文件名", "总页数", "转换页面范围", "状态", "操作", "删除"},
                    { 40, 380, 80, 165, 140, 115, 57 } };
        }
        case MasterModule::Type::PDFAction: {
            QStringList TextList{ "", "文件名", "总页数", "", "状态", "操作", "删除" };
            switch (qvariant_cast<PDFAction::Type>(sType))
            {
            case PDFAction::Type::PDFSplit:                 TextList[3] = "拆分页面范围"; break;
            case PDFAction::Type::PDFMerge:                 TextList[3] = "合并页面范围"; break;
            case PDFAction::Type::PDFCompress:              TextList[2] = "原大小"; TextList[3] = "压缩后大小"; break;
            case PDFAction::Type::PDFPageRenderAsImage:     TextList[3] = "转换页面范围"; break;
            case PDFAction::Type::PDFPageExtract:           TextList[3] = "提取页面范围"; break;
            case PDFAction::Type::PDFPageDelete:            TextList[3] = "删除页面范围"; break;
            case PDFAction::Type::PDFCrypto:                break;
            case PDFAction::Type::PDFWatermark:             TextList[3] = "添加水印范围"; break;
            case PDFAction::Type::DocumentTranslate:        TextList[3] = "翻译页面范围"; break;
            default: break;
            }
            return { .TextList = std::move(TextList),
               .ColumnWidthList{ 40, 380, 80, 165, 140, 115, 57 } };
        }
        case MasterModule::Type::ImageAction: {
            QStringList TextList{ "", "文件名", "图片尺寸", "", "状态", "操作", "删除" };
            switch (qvariant_cast<ImageAction::Type>(sType))
            {
            case ImageAction::Type::PDFToImage:                 TextList[2] = "总页数"; TextList[3] = "转换页面范围"; break;
            case ImageAction::Type::ImageToPDF:                 TextList[3] = "自定义尺寸"; break;
            case ImageAction::Type::ImageResize:                TextList[3] = "自定义尺寸"; break;
            case ImageAction::Type::ImageCompress:              TextList[2] = "原大小"; TextList[3] = "压缩后大小"; break;
            case ImageAction::Type::ImageTransfer:              break;
            case ImageAction::Type::ImageWatermark:             break;
            case ImageAction::Type::ImageOCR:                   break;
            case ImageAction::Type::ImageRotateCropFlip:        break;
            case ImageAction::Type::ImageAdvancedManipulation:  break;
            default: break;
            }
            return { .TextList = std::move(TextList),
               .ColumnWidthList{ 40, 310, 120, 165, 140, 135, 67 } };
        }
        }
        return { {}, {} };
    }
}

PWCentralWidget::PWCentralWidget(QWidget* parent)
    : QWidget(parent)
{
    _pLayer = new QStackedWidget(this);
    _pTableMask = new PWTableMaskWidget(this);
    _pTableView = new PWTableView(this);
    _pLayer->insertWidget(0, _pTableMask);
    _pLayer->insertWidget(1, _pTableView);
    _pLayer->setCurrentIndex(1);

    setObjectName("PWCentralWidget");
    PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_pLayer);

    QObject::connect(_pTableView, &PWTableView::switchClicked, this, &PWCentralWidget::switchToTableView);
    QObject::connect(_pTableMask, &PWTableMaskWidget::pressed, this, &PWCentralWidget::_openFileExplorer);
    QObject::connect(tableModel, &PWTableViewModel::rowsInserted, this, [this, tableModel]() {
        const PWTableViewModel::RowData& rowData = tableModel->getRowData(0);
        // 表格数据在添加之前已经过滤,确定 文件格式 正确匹配 模块
        // 仅 非Image模块 需要adjust调整"总页数"文本内容位置
        if (FileFormatType fileFormatType
            = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.CellData[1].toString() }.suffix());
            fileFormatType != FileFormatType::IMAGE)
        {
            _pTableView->adjustColummTextRect({ {1, {0, 0, 0, 0}},{2, {0, 0, -12, 0}} });
        }
        }, Qt::SingleShotConnection);


    // test data will be removed in the future
    QObject::connect(_pTableView, &NXTableView::tableViewShow, this, [this]() {

        PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
        // 第一行数据
        PWTableViewModel::RowData row1;
        row1.Index = 0;
        row1.State = FileStateType::BEREADY;
        row1.Checked = false;
        row1.CellData = QVariantList{
            QVariant(), // 第0列：复选框，不需要数据
            "E:/Mozilla-Recovery-Key_2025-07-28_2634544095@qq.com.pdf", // 第1列：文件名
            "25",       // 第2列：总页数
            QVariant::fromValue(_pTableView->createRangeWidget()), // 第3列：转换页面范围（无文本）
            QVariant::fromValue(_pTableView->createSwitchWidget()), // 第4列：输出格式（无文本）
            _pFileBereadyState, // 第5列：BEREADY状态文本,其他状态固定（通过State字段控制）
            QVariant(), // 第6列：操作（无文本）
            QVariant()  // 第7列：删除（无文本）
        };
        tableModel->appendRowData(row1);

        PWTableViewModel::RowData row2;
        row2.Index = 0;
        row2.State = FileStateType::BEREADY;
        row2.Checked = true;
        row2.CellData = QVariantList{
            QVariant(), // 第0列：复选框，不需要数据
            "E:/Project/小五编码-C++必知必会.doc", // 第1列：文件名
            "25",       // 第2列：总页数
           QVariant::fromValue(_pTableView->createRangeWidget()), // 第3列：转换页面范围（无文本）
           QVariant::fromValue(_pTableView->createSwitchWidget()), // 第4列：输出格式（无文本）
            _pFileBereadyState, // 第5列：BEREADY状态文本,其他状态固定（通过State字段控制）
            QVariant(), // 第6列：操作（无文本）
            QVariant()  // 第7列：删除（无文本）
        };
        tableModel->appendRowData(row2);
        }, Qt::SingleShotConnection);

}

PWCentralWidget::~PWCentralWidget()
{
}

void PWCentralWidget::setMask(const QPixmap& mask)
{
    _pTableMask->setMask(mask);
}

void PWCentralWidget::setModuleType(WizConverter::Enums::ModuleType ModuleType)
{
    _pModuleType = ModuleType;
    auto [headerTextList, columnWidthList] = Scope::Utils::GetHeaderViewMetadata(_pModuleType.MasterModuleType, _pModuleType.SlaveModuleType);
    _pTableView->setModuleType(_pModuleType);
    _pTableView->setHeaderTextList(headerTextList);
    _pTableView->setColumnWidthList(columnWidthList);
}

WizConverter::Enums::ModuleType PWCentralWidget::getModuleType() const
{
    return _pModuleType;
}

void PWCentralWidget::resizeEvent(QResizeEvent* event)
{
    _pLayer->setFixedSize(event->size());
    QWidget::resizeEvent(event);
}

void PWCentralWidget::_openFileExplorer()
{
    QStringList fileAbsolutePaths = QFileDialog::getOpenFileNames(this,
        "选择文件",
        QDir::homePath(),
        _pFileFilter);

    /*auto* tableWidget = _pTabWidgetInfos[_pIndexPair.first].at(_pIndexPair.second).TableWidget;
    QString fileState = _pTabWidgetInfos[_pIndexPair.first].at(_pIndexPair.second).FileState;
    qobject_cast<PWTableViewModel*>(tableWidget->model())->processDocumentData(fileAbsolutePaths, fileState);
    if (_pIndexPair.first != 1)
        Q_EMIT requestCreateAllTabBarWidgets(_pIndexPair.first, _pIndexPair.second);*/
}

void PWCentralWidget::addFiles(const QStringList& filePaths)
{
    // 实现添加文件到表格的逻辑
    // 这里可以根据实际需求实现具体的添加逻辑
    if (filePaths.isEmpty()) return;
    
    // 切换到表格视图
    _pLayer->setCurrentIndex(1);
    
    // 获取表格控件
    auto* tableWidget = qobject_cast<QTableWidget*>(_pLayer->widget(1));
    if (!tableWidget) return;
    
    // 添加文件到表格
    int currentRow = tableWidget->rowCount();
    for (const QString& filePath : filePaths) {
        tableWidget->insertRow(currentRow);
        QFileInfo fileInfo(filePath);
        tableWidget->setItem(currentRow, 0, new QTableWidgetItem(fileInfo.fileName()));
        tableWidget->setItem(currentRow, 1, new QTableWidgetItem(filePath));
        currentRow++;
    }
}

void PWCentralWidget::removeAll()
{
    if (!_pTableView || !_pTableView->model()->rowCount()) return;
    
    PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
    if (!tableModel) return;
    
    tableModel->onRemoveAllRows();
}

void PWCentralWidget::removeAllSelected()
{
    if (!_pTableView || !_pTableView->model()->rowCount()) return;

    PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
    if (!tableModel) return;

    tableModel->onRemoveSelectedRows();
}

void PWCentralWidget::removeAllNotSelected()
{
    if (!_pTableView || !_pTableView->model()->rowCount()) return;

    PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
    if (!tableModel) return;

    tableModel->onRemoveNotSelectedRows();
}