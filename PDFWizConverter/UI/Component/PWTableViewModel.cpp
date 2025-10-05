#include "PWTableViewModel.h"
#include <QIcon>
#include <QPixmap>
#include <QProcess>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>

#include "PWTableView.h"
#include "PWConverterWidget.h"
using namespace WizConverter::Enums;

PWTableViewModel::PWTableViewModel(QObject* parent)
    : QAbstractTableModel{ parent }
    , _pCheckedRowCount{ 0 }
{
    _pTableView = qobject_cast<PWTableView*>(this->parent());
    _pCheckIconList.reserve(5);
    _pFileTypeIconList.reserve((qsizetype)FileFormatType::__END);
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked.svg").scaled(TABLE_VIEW_CHECKICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Partially_Checked.svg").scaled(TABLE_VIEW_CHECKICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Checked.svg").scaled(TABLE_VIEW_CHECKICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked_Hover.svg").scaled(TABLE_VIEW_CHECKICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked_Press.svg").scaled(TABLE_VIEW_CHECKICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_CAD.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_CAJ.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_EPUB.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_EXCEL.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_HTML.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_IMAGE.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_MARKDOWN.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_OFD.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_PDF.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_POWERPOINT.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_TXT.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_WORD.png")));

}

PWTableViewModel::~PWTableViewModel()
{
}

int PWTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _pRowDataList.size();
}

int PWTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _pHeaderTextList.size();
}

QVariant PWTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= _pRowDataList.size() || index.column() >= _pHeaderTextList.size())
        return QVariant();

    const RowData& rowData = _pRowDataList.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole: {
        return _formatRowData(rowData, index.column());
    }
    case Qt::DecorationRole: {
        // 第0列显示复选框图标
        if (index.column() == 0) return rowData.Checked ? _pCheckIconList.at(Qt::Checked) : _pCheckIconList.at(Qt::Unchecked);
        // 第1列显示文件类型图标（根据文件扩展名）
        else if (index.column() == 1) {
            FileFormatType fileType = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.CellData.at(1).toString() }.suffix());
            return fileType == FileFormatType::__END ? QIcon() : _pFileTypeIconList.at(static_cast<int>(fileType));
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        if (index.column() == 1) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == 2) {
            FileFormatType fileType = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.CellData.at(1).toString() }.suffix());
            return QVariant::fromValue(fileType == FileFormatType::IMAGE ? Qt::AlignCenter : Qt::AlignRight | Qt::AlignVCenter);
        }
        return Qt::AlignCenter;
    }
                              // 已使用DecorationRole的svg图标作为复选框图标,这里不使用NXTabelView的CheckStateRole
                              /*case Qt::CheckStateRole:
                                  if (index.column() == 0) return rowData.Checked ? Qt::Checked : Qt::Unchecked;
                                  break;*/
    case Qt::DecorationPropertyRole: return Qt::AlignCenter;
    case Qt::UserRole:
        // 返回原始数据用于其他用途
        if (index.column() == 5) return QVariant::fromValue(rowData.State);
        break;
    }
    return QVariant();
}

bool PWTableViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= _pRowDataList.size() || index.column() >= _pHeaderTextList.size()) return false;
    RowData& rowData = _pRowDataList[index.row()];
    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        rowData.CellData[index.column()] = value;
        break;
    case Qt::CheckStateRole:
        if (index.column() == 0) rowData.Checked = value.toBool();
        break;
    case Qt::UserRole:
        rowData.State = qvariant_cast<FileStateType>(value);
        break;
    default: return false;
    }
    Q_EMIT dataChanged(index, index, { role });
    return true;
}

QVariant PWTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole: {
            if (section == 1 && rowCount() != 0)
            {
                return QString("全选      已选中(%1/%2)").arg(_pCheckedRowCount).arg(rowCount());
            }
            else if (section == columnCount() - 1)
            {
                return QVariant();
            }
            return _pHeaderTextList.empty() ? QVariant() : _pHeaderTextList.at(section);
        }
        case Qt::DecorationRole: {
            if (section == 0) {
                if (_pCheckedRowCount == 0) return _pCheckIconList.at(Qt::Unchecked);
                else if (_pCheckedRowCount < rowCount()) return _pCheckIconList.at(Qt::PartiallyChecked);
                else return _pCheckIconList.at(Qt::Checked);
            }
        }
        case Qt::TextAlignmentRole: {
            if (section == 1) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ForegroundRole: {
            if (section == columnCount() - 1) return QColor(0x19, 0x67, 0xC0);
        }
        default: break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


bool PWTableViewModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    return false;
}

void PWTableViewModel::setHeaderTextList(const QStringList& headerTextList)
{
    _pHeaderTextList = headerTextList;
}

void PWTableViewModel::setCellData(int row, const QVariantList& cellDatas)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    _pRowDataList[row].CellData = cellDatas;
    _pRowDataList[row].CellData.resize(columnCount());
    _updataCellIndexWidget(row, 3);

    // 后续可能需要修改拓展这个逻辑, 这里写死了
    if (columnCount() == 8) _updataCellIndexWidget(row, 4); // 仅 PDFToWord 模块

    // 通知整行数据改变
    QModelIndex topLeft = createIndex(row, 0);
    QModelIndex bottomRight = createIndex(row, columnCount() - 1);
    Q_EMIT dataChanged(topLeft, bottomRight);
}

void PWTableViewModel::setCellData(int row, QVariantList&& cellDatas)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    _pRowDataList[row].CellData = std::move(cellDatas);
    _pRowDataList[row].CellData.resize(columnCount());
    _updataCellIndexWidget(row, 3);
    if (columnCount() == 8) _updataCellIndexWidget(row, 4);

    QModelIndex topLeft = createIndex(row, 0);
    QModelIndex bottomRight = createIndex(row, columnCount() - 1);
    emit dataChanged(topLeft, bottomRight);
}

PWTableViewModel::RowData& PWTableViewModel::getRowData(int row)
{
    if (row < 0 || row >= _pRowDataList.size()) return _pRowDataList.front();
    return _pRowDataList[row];
}

void PWTableViewModel::setCellData(int row, int column, const QVariant& value)
{
    if (row < 0 || row >= _pRowDataList.size() || column < 0 || column >= _pHeaderTextList.size())
        return;

    RowData& rowData = _pRowDataList[row];

    if (rowData.CellData.size() <= column) rowData.CellData.resize(columnCount());
    rowData.CellData[column] = value;

    if ((column == 3 || column == 4) && value.isValid() && value.canConvert<NXModelIndexWidget*>()) {
        NXModelIndexWidget* widget = value.value<NXModelIndexWidget*>();
        if (widget) {
            QModelIndex newIndex = createIndex(row, column);
            widget->setIndex(newIndex);
            _pTableView->setIndexWidget(newIndex, widget);
        }
    }

    QModelIndex index = createIndex(row, column);
    Q_EMIT dataChanged(index, index);
}

QVariant PWTableViewModel::getCellData(int row, int column) const
{
    if (row < 0 || row >= _pRowDataList.size() || column < 0 || column >= _pHeaderTextList.size())
        return QVariant();

    const RowData& rowData = _pRowDataList[row];
    if (rowData.CellData.size() <= column) {
        return QVariant();
    }
    return rowData.CellData.at(column);
}

void PWTableViewModel::setRowData(const QList<RowData>& rowDataList)
{
    if (rowDataList.isEmpty()) return;

    beginResetModel();
    _pRowDataList = rowDataList;
    // 防止用户忘记设置索引, Checked, columnCount必须和表头一致
    _updateAllRowStates();
    // 更新所有 widget 的索引
    _updateAllWidgetsIndex();
    endResetModel();
}

void PWTableViewModel::setRowData(QList<RowData>&& rowDataList)
{
    if (rowDataList.isEmpty()) return;

    beginResetModel();
    _pRowDataList = std::move(rowDataList);
    _updateAllRowStates();
    _updateAllWidgetsIndex();
    endResetModel();
}

void PWTableViewModel::appendRowData(const RowData& rowData)
{
    beginInsertRows(QModelIndex(), _pRowDataList.size(), _pRowDataList.size());
    int newRow = _pRowDataList.size();
    RowData newRowData = rowData;
    newRowData.Index = newRow; // 设置新行的索引
    newRowData.CellData.resize(columnCount());
    _pRowDataList.append(std::move(newRowData));

    // 设置新行的 widget
    _setupWidgetForNewRow(newRow, _pRowDataList[newRow]);

    if (rowData.Checked) ++_pCheckedRowCount;
    endInsertRows();
}

void PWTableViewModel::appendRowData(RowData&& rowData)
{
    beginInsertRows(QModelIndex(), _pRowDataList.size(), _pRowDataList.size());
    int newRow = _pRowDataList.size();
    rowData.Index = newRow;
    rowData.CellData.resize(columnCount());
    _pRowDataList.append(std::move(rowData));

    _setupWidgetForNewRow(newRow, _pRowDataList[newRow]);

    if (rowData.Checked) ++_pCheckedRowCount;
    endInsertRows();
}

void PWTableViewModel::insertRowData(int row, const RowData& rowData)
{
    if (row < 0 || row > _pRowDataList.size()) return;

    beginInsertRows(QModelIndex(), row, row);
    RowData newRowData = rowData;
    newRowData.Index = row; // 设置插入行的索引
    newRowData.CellData.resize(columnCount());
    _pRowDataList.insert(row, std::move(newRowData));

    // 设置插入行的 widget
    _setupWidgetForNewRow(row, _pRowDataList[row]);

    // 更新插入位置之后所有行的索引和 widget
    _updateRowStatesFrom(row + 1);
    _updateRowWidgetsIndexFrom(row + 1);
    endInsertRows();
}

void PWTableViewModel::insertRowData(int row, RowData&& rowData)
{
    if (row < 0 || row > _pRowDataList.size()) return;

    beginInsertRows(QModelIndex(), row, row);
    rowData.Index = row;
    rowData.CellData.resize(columnCount());
    _pRowDataList.insert(row, std::move(rowData));

    _setupWidgetForNewRow(row, _pRowDataList[row]);

    _updateRowStatesFrom(row + 1);
    _updateRowWidgetsIndexFrom(row + 1);
    endInsertRows();
}

void PWTableViewModel::removeRowData(int row)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    bool wasChecked = _pRowDataList[row].Checked;

    beginRemoveRows(QModelIndex(), row, row);
    _pRowDataList.removeAt(row);
    _updateRowIndexesFrom(row);
    // 更新被删除行之后所有行的 widget 索引
    _updateRowWidgetsIndexFrom(row);
    endRemoveRows();

    if (wasChecked && _pCheckedRowCount > 0) {
        --_pCheckedRowCount;
    }

    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::removeRowData(const QModelIndex& index)
{
    removeRowData(index.row());
}

void PWTableViewModel::onSelectSingleRow(const QModelIndex& index)
{
    if (!index.isValid() || index.row() >= _pRowDataList.size())
        return;

    RowData& rowData = _pRowDataList[index.row()];
    bool oldChecked = rowData.Checked;
    rowData.Checked = !rowData.Checked;

    // 更新选中计数
    if (oldChecked && rowData.Checked) {
        // 不应该发生
    }
    else if (oldChecked && !rowData.Checked) {
        --_pCheckedRowCount;
    }
    else if (!oldChecked && rowData.Checked) {
        ++_pCheckedRowCount;
    }

    QModelIndex checkIndex = createIndex(index.row(), 0);
    Q_EMIT dataChanged(checkIndex, checkIndex, { Qt::DecorationRole });
    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::onSelectAllRows()
{
    std::for_each(_pRowDataList.begin(), _pRowDataList.end(), [this](RowData& rowData) {
        rowData.Checked = _pCheckedRowCount == 0;
        });
    _pCheckedRowCount = _pCheckedRowCount == 0 ? _pRowDataList.size() : 0;

    QModelIndex topLeft = createIndex(0, 0);
    QModelIndex bottomRight = createIndex(_pRowDataList.size() - 1, 0);
    Q_EMIT dataChanged(topLeft, bottomRight, { Qt::DecorationRole });
    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::onRemoveSingleRow(const QModelIndex& index)
{
    removeRowData(index);
}

void PWTableViewModel::onRemoveSelectedRows()
{
    if (_pCheckedRowCount == 0) return;
    // 从后往前删除，避免索引变化
    for (int i = _pRowDataList.size() - 1; i >= 0; --i) {
        if (_pRowDataList[i].Checked) {
            beginRemoveRows(QModelIndex(), i, i);
            _pRowDataList.removeAt(i);
            endRemoveRows();
        }
    }    
    // 删除完成后重新设置所有行的索引和 widget
    _updateAllRowIndexes();
    _updateAllWidgetsIndex();
    _pCheckedRowCount = 0;

    // 更新视图
    if (!_pRowDataList.isEmpty()) {
        QModelIndex topLeft = createIndex(0, 0);
        QModelIndex bottomRight = createIndex(_pRowDataList.size() - 1, columnCount() - 1);
        Q_EMIT dataChanged(topLeft, bottomRight);
    }

    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::onRemoveNotSelectedRows()
{
    if (_pCheckedRowCount == _pRowDataList.size()) return;

    for (int i = _pRowDataList.size() - 1; i >= 0; --i) {
        if (!_pRowDataList[i].Checked) {
            beginRemoveRows(QModelIndex(), i, i);
            _pRowDataList.removeAt(i);
            endRemoveRows();
        }
    }
    _updateAllRowIndexes();
    _updateAllWidgetsIndex();

    if (!_pRowDataList.isEmpty()) {
        QModelIndex topLeft = createIndex(0, 0);
        QModelIndex bottomRight = createIndex(_pRowDataList.size() - 1, columnCount() - 1);
        Q_EMIT dataChanged(topLeft, bottomRight);
    }

    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::onRemoveAllRows()
{
    if (_pRowDataList.isEmpty()) return;

    beginRemoveRows(QModelIndex(), 0, _pRowDataList.size() - 1);
    _pRowDataList.clear();
    endRemoveRows();
    _pCheckedRowCount = 0;

    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::onDelegateIconClicked(PWTableViewIconDelegate::IconRole role, const QModelIndex& index)
{
    using IconRole = PWTableViewIconDelegate::IconRole;
    if (!index.isValid() || index.row() >= _pRowDataList.size())
        return;
    const RowData& rowData = _pRowDataList[index.row()];
    if ((rowData.State & FileStateType::ERR) && role != IconRole::RemoveFile) {
        PWConverterWidget::ShowMessage("Error", "文件不存在或已损坏，请重新选择！", NXMessageBarType::Error, NXMessageBarType::TopLeft);
        return;
    }
    switch (role) {
    case IconRole::Run:
        // TODO: 执行当个文件转换
        break;
    case IconRole::OpenFile: QDesktopServices::openUrl(QUrl::fromLocalFile(rowData.CellData.at(1).toString()));break;
    case IconRole::OpenFolder: {
#if defined(Q_OS_WIN)
        QProcess::startDetached("explorer.exe", QStringList{} << "/select," << QDir::toNativeSeparators(rowData.CellData.at(1).toString()));
#elif defined(Q_OS_MAC)
        QProcess::startDetached("open", QStringList{} << "-R" << QDir::toNativeSeparators(rowData.CellData.at(1).toString()));
#elif defined(Q_OS_LINUX)
        QStringList fileManagers = {
            "nautilus", "dolphin", "thunar", "caja", "nemo", "pcmanfm", "kfmclient"
        };
        for (const QString& manager : fileManagers) {
            if (QProcess::execute("which", QStringList() << manager) == 0) {
                QStringList args;

                // 如果指定了选中文件，尝试使用相应参数
                if (manager == "nautilus" || manager == "nemo" || manager == "dolphin") {
                    args << "--select" << selectFile;
                }
                // Thunar不支持直接选中文件，只打开文件夹
                else if (manager == "thunar") args << folderPath;
                else if (manager == "kfmclient")  args << "exec" << selectFile;
                else args << folderPath;

                if (QProcess::startDetached(manager, args)) return;
            }
        }
        // 如果所有文件管理器都失败，使用xdg-open
        QProcess::startDetached("xdg-open", QStringList{} << QDir::toNativeSeparators(rowData.CellData.at(1).toString()));
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(rowData.CellData.at(1).toString()));
#endif
        break;
    }
    case IconRole::RemoveFile: removeRowData(index.row()); break;
    default: break;
    }
}

QVariant PWTableViewModel::_formatRowData(const RowData& rowData, int column) const
{
    const QString& headerText = _pHeaderTextList.at(column);
    if (headerText == "原大小" || headerText == "压缩后大小") {
        return QLocale().formattedDataSize(rowData.CellData.at(column).toULongLong(), 2, QLocale::DataSizeSIFormat);
    }
    switch (column) {
    case 1: { // 文件名
        if (rowData.CellData.at(1).isNull()) return QVariant();
        const QString& fileAbsolutePath = rowData.CellData.at(1).toString();
        QFileInfo fileInfo(fileAbsolutePath);
        const QString& fileName = fileInfo.fileName();

        QFontMetrics metrics(_pTableView->font());

        int cellWidth = _pTableView->columnWidth(column);
        int availableWidth = cellWidth - 118;
        if (fileInfo.isFile() && fileInfo.exists()) {
            if (metrics.horizontalAdvance(fileName) > availableWidth) {
                return metrics.elidedText(fileName, Qt::ElideMiddle, availableWidth); // 返回裁剪后的文本
            }
            return fileName; // 默认返回未裁剪的文本
        }
        else {
            const_cast<PWTableViewModel*>(this)->_pRowDataList[rowData.Index].State = FileStateType::DELETED;
            return fileAbsolutePath;
        }
    }
    case 2: // 总页数 or 图片尺寸
        return rowData.CellData.at(2).isNull() ? QVariant() : rowData.CellData.at(2);
    case 5: { // 状态
        if (rowData.CellData.at(5).isNull()) return QVariant();
        switch (static_cast<FileStateType>(rowData.State.toInt())) {
        case FileStateType::LOADING: return "加载中";
        case FileStateType::BEREADY: return rowData.CellData.at(5); // 由JSON配置设置
        case FileStateType::PROCESSING: return "转换中";
        case FileStateType::SUCCESS: return "已完成";
        case FileStateType::FAILED: return "失败";
        case FileStateType::CORRUPTION: return "文件损坏";
        case FileStateType::DELETED: return "文件已删除";
        case FileStateType::UNKONWERROR: return "未知错误";
        default: return "未知状态";
        }
    }
    default: return QVariant(); // 其他列不显示文本
    }
}

void PWTableViewModel::_updateRowIndexesFrom(int startRow)
{
    for (int i = startRow; i < _pRowDataList.size(); ++i) {
        _pRowDataList[i].Index = i;
    }
}

void PWTableViewModel::_updateRowStatesFrom(int startRow)
{
    for (int i = startRow; i < _pRowDataList.size(); ++i) {
        _pRowDataList[i].Index = i;
        if(_pRowDataList[i].Checked) ++_pCheckedRowCount;
    }
}

void PWTableViewModel::_updateAllRowIndexes()
{
    for (int i = 0; i < _pRowDataList.size(); ++i) {
        _pRowDataList[i].Index = i;
    }
}

void PWTableViewModel::_updateAllRowStates()
{
    _pCheckedRowCount = 0;
    for (int i = 0; i < _pRowDataList.size(); ++i) {
        _pRowDataList[i].Index = i;
        if (auto& cellData = _pRowDataList[i].CellData; cellData.size() != columnCount()) cellData.resize(columnCount());
        if (_pRowDataList[i].Checked) ++_pCheckedRowCount;
    }
}

void PWTableViewModel::_updataCellIndexWidget(int row, int column)
{   // 更新该行的 widget 索引
    if (_pRowDataList[row].CellData.size() > column) {
        QVariant widgetVariant = _pRowDataList[row].CellData[column];
        if (widgetVariant.isValid() && widgetVariant.canConvert<NXModelIndexWidget*>()) {
            NXModelIndexWidget* widget = widgetVariant.value<NXModelIndexWidget*>();
            if (widget) {
                QModelIndex newIndex = createIndex(row, column);
                widget->setIndex(newIndex);
                _pTableView->setIndexWidget(newIndex, widget);
            }
        }
    }
}

void PWTableViewModel::_updateRowWidgetsIndexFrom(int startRow)
{
    for (int row = startRow; row < _pRowDataList.size(); ++row) {
        _updataCellIndexWidget(row, 3);
        if (columnCount() == 8) _updataCellIndexWidget(row, 4);
    }
}

void PWTableViewModel::_updateAllWidgetsIndex()
{
    for (int row = 0; row < _pRowDataList.size(); ++row) {
        _updataCellIndexWidget(row, 3);
        if (columnCount() == 8) _updataCellIndexWidget(row, 4);
    }
}

void PWTableViewModel::_setupWidgetForNewRow(int row, RowData& rowData)
{
    // 没有足够的列
    if (columnCount() <= 3)  return;

    auto createWidgetFromColumnFunc = [&](int column) {
        // 创建新的 widget 或更新现有 widget 的索引
        QVariant widgetVariant = rowData.CellData[column];
        NXModelIndexWidget* widget = nullptr;

        if (widgetVariant.isValid() && widgetVariant.canConvert<NXModelIndexWidget*>()) {
            widget = widgetVariant.value<NXModelIndexWidget*>();
        }
        else {
            // 创建新的 widget
            widget = new NXModelIndexWidget(_pTableView);
            rowData.CellData[column] = QVariant::fromValue(widget);
        }

        if (widget) {
            QModelIndex newIndex = createIndex(row, column);
            widget->setIndex(newIndex);

            _pTableView->setIndexWidget(newIndex, widget);
        }
        };

    createWidgetFromColumnFunc(3);
    if (columnCount() == 8) createWidgetFromColumnFunc(4);
}
