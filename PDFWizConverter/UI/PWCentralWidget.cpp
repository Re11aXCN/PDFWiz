#include "PWCentralWidget.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QResizeEvent>
#include <QFileInfo>
#include <QTimer>

#include "Component/PWTableMaskWidget.h"
#include "Component/PWTableView.h"
#include "Component/PWTableViewModel.h"
#include "Component/PWHeaderView.h"
using namespace WizConverter::Enums;
namespace Scope::Utils {
    struct HeaderViewMetadata {
        QStringList TextList;
        QList<int> ColumnWidthList;
    };
    static HeaderViewMetadata GetHeaderViewMetadata(MasterModule::Type mType, const QVariant& sType) {
        switch (mType) {
        case MasterModule::Type::PDFToWord: {
            return { {"", "文件名", "总页数", "转换页面范围", "输出格式", "状态", "操作", ""},
                { 40, 300, 60, 145, 120, 140, 115, 57 } };
        }
        case MasterModule::Type::WordToPDF: {
            return qvariant_cast<WordToPDF::Type>(sType) == WordToPDF::Type::ImageToPDF
                ? HeaderViewMetadata{ {"", "文件名", "图片尺寸", "自定义尺寸", "状态", "操作", ""},
                    { 40, 310, 120, 165, 140, 135, 67 } }
                : HeaderViewMetadata{ {"", "文件名", "总页数", "转换页面范围", "状态", "操作", ""},
                    { 40, 380, 80, 165, 140, 115, 57 } };
        }
        case MasterModule::Type::PDFAction: {
            QStringList TextList{ "", "文件名", "总页数", "", "状态", "操作", "" };
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
            QStringList TextList{ "", "文件名", "图片尺寸", "", "状态", "操作", "" };
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
    _pTableViewModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
    _pLayer->insertWidget(0, _pTableMask);
    _pLayer->insertWidget(1, _pTableView);
    _pLayer->setCurrentIndex(0);

    setObjectName("PWCentralWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_pLayer);

    QObject::connect(_pTableView, &PWTableView::switchClicked, this, &PWCentralWidget::switchToTableView);
    QObject::connect(_pTableMask, &PWTableMaskWidget::pressed, this, &PWCentralWidget::_openFileExplorer);
    QObject::connect(_pTableViewModel, &PWTableViewModel::rowsRemoved, this, [this]() {
        if (!_pTableViewModel->rowCount()) {
            _pLayer->setCurrentIndex(0);
        }
        });
    QObject::connect(_pTableViewModel, &PWTableViewModel::rowsInserted, this, [this]() {
        const PWTableViewModel::RowData& rowData = _pTableViewModel->getRowData(0);
        // 表格数据在添加之前已经过滤,确定 文件格式 正确匹配 模块
        // 仅 非Image模块 需要adjust调整"总页数"文本内容位置
        if (FileFormatType fileFormatType
            = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.FileName }.suffix());
            fileFormatType != FileFormatType::IMAGE)
        {
            _pTableView->adjustColummTextRect({ {1, {0, 0, 0, 0}},{2, {0, 0, -12, 0}} });
        }
        }, Qt::SingleShotConnection);

    //QObject::connect(_pTableView, &NXTableView::tableViewShow, this, [this]() {
        //PWTableViewModel* tableModel = qobject_cast<PWTableViewModel*>(_pTableView->model());
        //PWTableViewModel::RowData rowData;
        //rowData.Index = 0;
        //rowData.Checked = false;
        //rowData.FileName = "E:/Mozilla-Recovery-Key_2025-07-28_2634544095@qq.com.pdf";
        //rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 25}}, 25} });
        //rowData.RangeWidget = _pTableView->createRangeWidget();
        //rowData.SwitchWidget = _pTableView->createSwitchWidget(); // 8列表格特有
        //rowData.FileProcessState = PWTableViewModel::FileProcessState{ FileStateType::BEREADY, _pFileBereadyState };

        //tableModel->appendRowData(rowData);

        //PWTableViewModel::RowData row2;
        //row2.Index = 1;
        //row2.Checked = true;
        //row2.FileName = "E:/Project/小五编码-C++必知必会.doc";
        //row2.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 20}}, 20} });
        //row2.RangeWidget = _pTableView->createRangeWidget();
        //row2.SwitchWidget = _pTableView->createSwitchWidget(); // 8列表格特有
        //row2.FileProcessState = PWTableViewModel::FileProcessState{ FileStateType::BEREADY, _pFileBereadyState };
        //tableModel->appendRowData(row2);
        //}, Qt::SingleShotConnection);
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
    auto [textList, columnWidthList] = Scope::Utils::GetHeaderViewMetadata(_pModuleType.MasterModuleType, _pModuleType.SlaveModuleType);
    _pTableView->setModuleType(_pModuleType);
    _pTableView->setHeaderTextList(textList);
    _pTableView->setColumnWidthList(columnWidthList);

    addFiles({ "C:/Users/Re11a/Pictures/miku01.jpg" });
    _pLayer->setCurrentIndex(1);
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

void PWCentralWidget::addFiles(const QStringList& filePaths)
{
#define FOR_START  for (const auto& filePath : filePaths) {\
    PWTableViewModel::RowData rowData;\
    rowData.Index = _pTableViewModel->rowCount();\
    rowData.Checked = true;\
    rowData.FileName = filePath;\
    rowData.FileProcessState = PWTableViewModel::FileProcessState{ FileStateType::BEREADY, _pFileBereadyState };

#define FOR_END rowDataList.append(rowData); }

    if (filePaths.isEmpty() || !_pTableView || !_pTableViewModel) return;
    QTimer::singleShot(200, this, [this]() {
        _pLayer->setCurrentIndex(1);
        });
    QList<PWTableViewModel::RowData> rowDataList;
    rowDataList.reserve(filePaths.size());
    if (_pModuleType.MasterModuleType == MasterModule::Type::PDFToWord)
    {
        FOR_START
            // TODO: CALL C# TO GET PAGE COUNT
            rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 20}}, 20} });
            rowData.RangeWidget = _pTableView->createRangeWidget();
            rowData.SwitchWidget = _pTableView->createSwitchWidget();
            // TODO: CALL C# TO SAVA PASSWORD ISHASIMAGE
            rowData.ExpandData = {};
        FOR_END
    }
    else if(_pModuleType.MasterModuleType == MasterModule::Type::PDFAction
        || qvariant_cast<ImageAction::Type>(_pModuleType.SlaveModuleType) == ImageAction::Type::PDFToImage)
    {
        FOR_START
            // TODO: CALL C# TO GET PAGE COUNT
            rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 20}}, 20} });
            rowData.RangeWidget = _pTableView->createRangeWidget();
            // TODO: CALL C# TO SAVA PASSWORD ISHASIMAGE
            rowData.ExpandData = {};
        FOR_END
    }
    else if (_pModuleType.MasterModuleType == MasterModule::Type::ImageAction
        || qvariant_cast<WordToPDF::Type>(_pModuleType.SlaveModuleType) == WordToPDF::Type::ImageToPDF)
    {
        FOR_START
            QImage image(filePath);
            rowData.setImageSizeData({ .OriginalSize = image.size(), .ResizedSize = image.size() });
            rowData.RangeWidget = _pTableView->createRangeWidget();
        FOR_END
    }
    else {
        switch (_pModuleType.SlaveModuleType.toInt()) {
        case static_cast<int>(WordToPDF::Type::WordToPDF): {
            FOR_START
                // TODO: CALL C# TO GET PAGE COUNT
                rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 1}}, 1} });
                rowData.RangeWidget = _pTableView->createRangeWidget();
            FOR_END
        }break;
        case static_cast<int>(WordToPDF::Type::ExcelToPDF): {
            FOR_START
                // TODO: CALL C# TO GET PAGE COUNT
                rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 1}}, 1} });
                rowData.RangeWidget = _pTableView->createRangeWidget();
            FOR_END
        }break;
        case static_cast<int>(WordToPDF::Type::PowerpointToPDF): {
            FOR_START
                // TODO: CALL C# TO GET PAGE COUNT
                rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 1}}, 1} });
                rowData.RangeWidget = _pTableView->createRangeWidget();
            FOR_END
        }break;
        default: {
            FOR_START
                rowData.setRangeData({ PWTableViewModel::RangeData{ {PWTableViewModel::RangeData::Range{1, 1}}, 1} });
                rowData.RangeWidget = _pTableView->createRangeWidget();
            FOR_END
        }break;
        }
        
    }
    _pTableViewModel->appendRowData(std::move(rowDataList));
}

void PWCentralWidget::removeAll()
{
    if (!_pTableView || !_pTableViewModel || !_pTableView->model()->rowCount()) return;
    
    _pTableViewModel->onRemoveAllRows();

    _pLayer->setCurrentIndex(0);
}

void PWCentralWidget::removeAllSelected()
{
    if (!_pTableView || !_pTableViewModel || !_pTableView->model()->rowCount()) return;

    _pTableViewModel->onRemoveSelectedRows();

    if(!_pTableViewModel->rowCount()) _pLayer->setCurrentIndex(0);
}

void PWCentralWidget::removeAllNotSelected()
{
    if (!_pTableView || !_pTableViewModel || !_pTableView->model()->rowCount()) return;

    _pTableViewModel->onRemoveNotSelectedRows();

    if (!_pTableViewModel->rowCount()) _pLayer->setCurrentIndex(0);
}

void PWCentralWidget::_openFileExplorer()
{
    QStringList fileAbsolutePaths = QFileDialog::getOpenFileNames(this,
        "选择文件",
        QDir::homePath(),
        _pFileFilter);
    if (fileAbsolutePaths.isEmpty()) return;
    addFiles(fileAbsolutePaths);
}

