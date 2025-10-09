#include "PWTableViewModel.h"
#include <QIcon>
#include <QPixmap>
#include <QProcess>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMimeData>
#include <NXLineEdit.h>
#include <magic_enum/magic_enum.hpp>
#include "PWTableView.h"
#include "PWConverterWidget.h"
using namespace WizConverter::Enums;

namespace Scope::Utils {
    QList<std::pair<int, int>> DedupAndGetRanges(QList<int> nums) {
        QList<std::pair<int, int>> ranges;

        if (nums.empty()) return ranges;

        // 1. 先排序
        std::sort(nums.begin(), nums.end());

        // 2. 原地去重
        int n = nums.size();
        int write_idx = 0;
        for (int read_idx = 0; read_idx < n; read_idx++) {
            if (read_idx == 0 || nums[read_idx] != nums[read_idx - 1]) {
                nums[write_idx++] = nums[read_idx];
            }
        }
        // 调整vector大小，释放多余空间
        nums.resize(write_idx);

        // 3. 找出连续区间
        n = nums.size();
        int start = nums[0];

        for (int i = 1; i < n; i++) {
            if (nums[i] != nums[i - 1] + 1) {
                // 当前区间结束
                ranges.emplace_back(start, nums[i - 1]);
                start = nums[i];
            }
        }
        // 处理最后一个区间
        ranges.emplace_back(start, nums[n - 1]);

        return ranges;
    }
}

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

Qt::ItemFlags PWTableViewModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid()) {
        // 使项目可拖动、可放置、可选择、可启用
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    else {
        // 对于无效的索引（如表格空白区域），允许放置
        return defaultFlags | Qt::ItemIsDropEnabled;
    }
}

Qt::DropActions PWTableViewModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions PWTableViewModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList PWTableViewModel::mimeTypes() const
{
    return QStringList() << "application/x-pwtableviewmodel-row";
}

QMimeData* PWTableViewModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.isEmpty()) return nullptr;
    QMimeData* mimeData = new QMimeData;
    QModelIndex index = indexes.first();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << index.row() << index.column();

    mimeData->setData("application/x-pwtableviewmodel-row", data);
    return mimeData;
}

bool PWTableViewModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/x-pwtableviewmodel-row") ||
        action != Qt::MoveAction ||
        row < 0 || row > _pRowDataList.size())
        return false;

    return true;
}

bool PWTableViewModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction) return true;

    QByteArray encodedData = data->data("application/x-pwtableviewmodel-row");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    const int& targetRow = row;
    int draggedRow, draggedColumn;
    stream >> draggedRow >> draggedColumn;
    int dropIndicatorPos = _pTableView->property("PWDropIndicatorPosition").toInt();

    auto moveRowData = [this, draggedRow, column](int insertPos, int startMovePos, int endMovePos, bool moveForward) {
        RowData draggedData = _pRowDataList[draggedRow];

        if (moveForward) {
            // 向前移动数据
            for (int i = startMovePos; i < endMovePos; ++i) {
                _pRowDataList[i] = _pRowDataList[i + 1];
            }
        }
        else {
            // 向后移动数据
            for (int i = endMovePos; i > startMovePos; --i) {
                _pRowDataList[i] = _pRowDataList[i - 1];
            }
        }

        // 插入拖拽的数据
        _pRowDataList[insertPos] = draggedData;

        // 更新行数据和发送信号
        int affectedStart = moveForward ? qMin(draggedRow, insertPos) : qMin(startMovePos, insertPos);
        int affectedEnd = moveForward ? qMax(draggedRow, insertPos) : qMax(endMovePos, insertPos);

        _updateMoveActionAllRowData(affectedStart, affectedEnd);
        Q_EMIT dataChanged(index(affectedStart, 0), index(affectedEnd, columnCount() - 1));
        };

    // 处理不同的移动情况
    if (targetRow < draggedRow) {
        if (dropIndicatorPos == 1/*AboveItem*/) {
            moveRowData(targetRow, targetRow, draggedRow, false);
        }
        else if (dropIndicatorPos == 2/*BelowItem*/) {
            moveRowData(targetRow + 1, targetRow + 1, draggedRow, false);
        }
    }
    else if (targetRow > draggedRow) {
        if (dropIndicatorPos == 1/*AboveItem*/) {
            moveRowData(targetRow - 1, draggedRow, targetRow - 1, true);
        }
        else if (dropIndicatorPos == 2/*BelowItem*/) {
            moveRowData(targetRow, draggedRow, targetRow, true);
        }
    }

    return true;
}

QVariant PWTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= _pRowDataList.size() || index.column() >= _pHeaderTextList.size())
        return QVariant();

    const RowData& rowData = _pRowDataList.at(index.row());
    if (property("IsGridViewMode").toBool())
    {
        switch (role)
        {
        case Qt::UserRole: {
            return rowData.FileName;
        }
        case Qt::UserRole + 1: {
            return property("IsGridViewMode").toBool();
        }
        case Qt::UserRole + 2: {
            if (index.column() == (_isEightColumnTable() ? 5 : 4)) return QVariant::fromValue(rowData.FileProcessState.State);
            break;
        }
        }
        return QVariant();
    }
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
            FileFormatType fileType = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.FileName }.suffix());
            return fileType == FileFormatType::__END ? QIcon() : _pFileTypeIconList.at(static_cast<int>(fileType));
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        if (index.column() == 1) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == 2) {
            FileFormatType fileType = FileFormat::GetFileTypeByExtension(QFileInfo{ rowData.FileName }.suffix());
            return QVariant::fromValue(fileType == FileFormatType::IMAGE ? Qt::AlignCenter : Qt::AlignRight | Qt::AlignVCenter);
        }
        return Qt::AlignCenter;
    }
    case Qt::DecorationPropertyRole: return Qt::AlignCenter;
    }
    return QVariant();
}

bool PWTableViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= _pRowDataList.size() || index.column() >= _pHeaderTextList.size())
        return false;

    RowData& rowData = _pRowDataList[index.row()];
    switch (role)
    {
    case Qt::CheckStateRole: {
        if (index.column() == 0) {
            bool oldChecked = rowData.Checked;
            rowData.Checked = value.toBool();

            // 更新选中计数
            if (oldChecked && !rowData.Checked && _pCheckedRowCount > 0) {
                --_pCheckedRowCount;
            }
            else if (!oldChecked && rowData.Checked) {
                ++_pCheckedRowCount;
            }
        }
        break;
    }
    case Qt::UserRole: {
        if (index.column() == (_isEightColumnTable() ? 5 : 4)) {
            rowData.FileProcessState.State = qvariant_cast<FileStateType>(value);
        }
        break;
    }
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
            if (property("IsGridViewMode").toBool()) {
                if (section == 0 && _pRowDataList.size() != 0) {
                    return QString("           全选      已选中(%1/%2)").arg(_pCheckedRowCount).arg(_pRowDataList.size());
                }
                return _pHeaderTextList.empty() ? QVariant() : _pHeaderTextList.at(section);
            }
            else {
                if (section == 1 && _pRowDataList.size() != 0)
                {
                    return QString("全选      已选中(%1/%2)").arg(_pCheckedRowCount).arg(_pRowDataList.size());
                }
                return _pHeaderTextList.empty() ? QVariant() : _pHeaderTextList.at(section);
            }
        }
        case Qt::DecorationRole: {
            if (section == 0) {
                if (_pCheckedRowCount == 0) return _pCheckIconList.at(Qt::Unchecked);
                else if (_pCheckedRowCount < _pRowDataList.size()) return _pCheckIconList.at(Qt::PartiallyChecked);
                else return _pCheckIconList.at(Qt::Checked);
            }
        }
        case Qt::TextAlignmentRole: {
            if (property("IsGridViewMode").toBool() && section == 0) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
            if (section == 1) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ForegroundRole: {
            if (section == columnCount() - 1 || property("IsGridViewMode").toBool()) return QColor(0x19, 0x67, 0xC0);
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

QStringList PWTableViewModel::getHeaderTextList() const
{
    return _pHeaderTextList;
}

void PWTableViewModel::setRowData(const QList<RowData>& rowDataList)
{
    if (rowDataList.isEmpty()) {
        appendRowData(rowDataList);
        return;
    }

    beginResetModel();
    _pRowDataList = rowDataList;
    endResetModel();
    _updateResetActionAllRowData();
}

void PWTableViewModel::setRowData(QList<RowData>&& rowDataList)
{
    if (rowDataList.isEmpty()) {
        appendRowData(rowDataList);
        return;
    }

    beginResetModel();
    _pRowDataList = std::move(rowDataList);
    endResetModel();
    _updateResetActionAllRowData();
}

PWTableViewModel::RowData& PWTableViewModel::getRowData(int row)
{
    if (row < 0 || row >= _pRowDataList.size()) return _pRowDataList.front();
    return _pRowDataList[row];
}

const PWTableViewModel::RowData& PWTableViewModel::getRowData(int row) const
{
    if (row < 0 || row >= _pRowDataList.size())
        return _pRowDataList.front();
    return _pRowDataList[row];
}

void PWTableViewModel::appendRowData(const RowData& rowData)
{
    beginInsertRows(QModelIndex(), _pRowDataList.size(), _pRowDataList.size());
    int newRow = _pRowDataList.size();
    RowData newRowData = rowData;
    newRowData.Index = newRow;
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
    _pRowDataList.insert(row, std::move(newRowData));

    // 设置插入行的 widget
    _setupWidgetForNewRow(row, _pRowDataList[row]);

    // 更新插入位置之后所有行的索引和 widget
    _updateIRActionRowDataFrom(row + 1);
    endInsertRows();
}

void PWTableViewModel::insertRowData(int row, RowData&& rowData)
{
    if (row < 0 || row > _pRowDataList.size()) return;

    beginInsertRows(QModelIndex(), row, row);
    rowData.Index = row;
    _pRowDataList.insert(row, std::move(rowData));

    _setupWidgetForNewRow(row, _pRowDataList[row]);

    _updateIRActionRowDataFrom(row + 1);
    endInsertRows();
}

void PWTableViewModel::removeRowData(int row)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    bool wasChecked = _pRowDataList[row].Checked;

    beginRemoveRows(QModelIndex(), row, row);
    _pRowDataList.removeAt(row);
    // 更新被删除行之后所有行的 widget 索引
    _updateIRActionRowDataFrom(row);
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

void PWTableViewModel::appendRowData(const QList<RowData>& rowDataList)
{
    if (rowDataList.isEmpty()) return;

    beginInsertRows(QModelIndex(), _pRowDataList.size(), _pRowDataList.size() + rowDataList.size() - 1);
    int startRow = _pRowDataList.size();

    for (int i = 0; i < rowDataList.size(); ++i) {
        RowData newRowData = rowDataList[i];
        newRowData.Index = startRow + i;
        _pRowDataList.append(std::move(newRowData));

        // 设置新行的 widget
        _setupWidgetForNewRow(startRow + i, _pRowDataList[startRow + i]);

        if (rowDataList[i].Checked) ++_pCheckedRowCount;
    }

    endInsertRows();
}

void PWTableViewModel::appendRowData(QList<RowData>&& rowDataList)
{
    if (rowDataList.isEmpty()) return;

    beginInsertRows(QModelIndex(), _pRowDataList.size(), _pRowDataList.size() + rowDataList.size() - 1);
    int startRow = _pRowDataList.size();

    for (int i = 0; i < rowDataList.size(); ++i) {
        rowDataList[i].Index = startRow + i;
        _pRowDataList.append(std::move(rowDataList[i]));

        _setupWidgetForNewRow(startRow + i, _pRowDataList[startRow + i]);

        if (rowDataList[i].Checked) ++_pCheckedRowCount;
    }

    endInsertRows();
}

void PWTableViewModel::insertRowData(int row, const QList<RowData>& rowDataList)
{
    if (row < 0 || row > _pRowDataList.size() || rowDataList.isEmpty()) return;

    beginInsertRows(QModelIndex(), row, row + rowDataList.size() - 1);

    // 插入数据
    for (int i = 0; i < rowDataList.size(); ++i) {
        RowData newRowData = rowDataList[i];
        newRowData.Index = row + i;
        _pRowDataList.insert(row + i, std::move(newRowData));

        // 设置插入行的 widget
        _setupWidgetForNewRow(row + i, _pRowDataList[row + i]);

        if (rowDataList[i].Checked) ++_pCheckedRowCount;
    }

    // 更新插入位置之后所有行的索引和 widget
    _updateIRActionRowDataFrom(row + rowDataList.size());

    endInsertRows();
}

void PWTableViewModel::insertRowData(int row, QList<RowData>&& rowDataList)
{
    if (row < 0 || row > _pRowDataList.size() || rowDataList.isEmpty()) return;

    beginInsertRows(QModelIndex(), row, row + rowDataList.size() - 1);

    for (int i = 0; i < rowDataList.size(); ++i) {
        rowDataList[i].Index = row + i;
        _pRowDataList.insert(row + i, std::move(rowDataList[i]));

        _setupWidgetForNewRow(row + i, _pRowDataList[row + i]);

        if (rowDataList[i].Checked) ++_pCheckedRowCount;
    }

    _updateIRActionRowDataFrom(row + rowDataList.size());

    endInsertRows();
}

void PWTableViewModel::removeRowData(QList<int>&& rows)
{
    if (rows.isEmpty()) return;
    auto ranges = Scope::Utils::DedupAndGetRanges(std::move(rows));
    for (auto it = ranges.rbegin(); it != ranges.rend(); ++it) {
        beginRemoveRows(QModelIndex(), (*it).first, (*it).second);
        for (int row = (*it).second; row >= (*it).first; --row) {
            if(_pRowDataList[row].Checked) --_pCheckedRowCount;
            _pRowDataList.removeAt(row);
        }
        endRemoveRows();
    }
    _updateIRActionAllRowData();
    if (!_pRowDataList.isEmpty()) {
        QModelIndex topLeft = createIndex(0, 0);
        QModelIndex bottomRight = createIndex(_pRowDataList.size() - 1, columnCount() - 1);
        Q_EMIT dataChanged(topLeft, bottomRight);
    }
    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

void PWTableViewModel::removeRowData(const QModelIndexList& indexes)
{
    if (indexes.isEmpty()) return;

    QList<int> rows;
    rows.reserve(indexes.size());

    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            rows.append(index.row());
        }
    }

    removeRowData(std::move(rows));
}

QList<PWTableViewModel::RowData> PWTableViewModel::getSelectedRows() const
{
    QList<RowData> selectedRows;
    selectedRows.reserve(_pCheckedRowCount);
    for (const auto& rowData : _pRowDataList) {
        if (rowData.Checked) {
            selectedRows.append(rowData);
        }
    }
    return selectedRows;
}

QList<int> PWTableViewModel::getSelectedRowIndexes() const
{
    QList<int> indexes;
    indexes.reserve(_pCheckedRowCount);
    for (int i = 0; i < _pRowDataList.size(); ++i) {
        if (_pRowDataList[i].Checked) {
            indexes.append(i);
        }
    }
    return indexes;
}

void PWTableViewModel::updateRowState(int row, FileStateType state, const QString& text)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    _pRowDataList[row].FileProcessState.State = state;
    if (!text.isEmpty()) {
        _pRowDataList[row].FileProcessState.Text = text;
    }

    int stateColumn = _isEightColumnTable() ? 5 : 4;
    QModelIndex index = createIndex(row, stateColumn);
    Q_EMIT dataChanged(index, index);
}

void PWTableViewModel::updateRowRangeData(int row, const RangeData& range)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    _pRowDataList[row].setRangeData(range);
    QModelIndex index = createIndex(row, 2);
    Q_EMIT dataChanged(index, index);
}

void PWTableViewModel::updateRowSize(int row, const ImageSizeData& size)
{
    if (row < 0 || row >= _pRowDataList.size()) return;

    _pRowDataList[row].setImageSizeData(size);
    QModelIndex index = createIndex(row, 2);
    Q_EMIT dataChanged(index, index);
}

void PWTableViewModel::resetRemoveIndexWidgits()
{
    beginResetModel();
    endResetModel();
}

void PWTableViewModel::resetRecoverIndexWidgits()
{
    _updateResetActionAllRowData();
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
    _updateIRActionAllRowData();
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
    _updateIRActionAllRowData();

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
    if (!index.isValid() || index.row() >= _pRowDataList.size()) return;

    const RowData& rowData = _pRowDataList[index.row()];
    if ((rowData.FileProcessState.State & FileStateType::ERR) && role != IconRole::RemoveFile) {
        PWConverterWidget::ShowMessage("Error", "文件不存在或已损坏，请重新选择！", NXMessageBarType::Error, NXMessageBarType::TopLeft);
        return;
    }
    switch (role) {
    case IconRole::Run:
        // TODO: 执行单个文件转换
        break;
    case IconRole::OpenFile: QDesktopServices::openUrl(QUrl::fromLocalFile(rowData.FileName)); break;
    case IconRole::OpenFolder: {
#if defined(Q_OS_WIN)
        QProcess::startDetached("explorer.exe", QStringList{} << "/select," << QDir::toNativeSeparators(rowData.FileName));
#elif defined(Q_OS_MAC)
        QProcess::startDetached("open", QStringList{} << "-R" << QDir::toNativeSeparators(rowData.FileName));
#elif defined(Q_OS_LINUX)
        QStringList fileManagers = {
            "nautilus", "dolphin", "thunar", "caja", "nemo", "pcmanfm", "kfmclient"
        };
        for (const QString& manager : fileManagers) {
            if (QProcess::execute("which", QStringList() << manager) == 0) {
                QStringList args;

                // 如果指定了选中文件，尝试使用相应参数
                if (manager == "nautilus" || manager == "nemo" || manager == "dolphin") {
                    args << "--select" << rowData.FileName;
                }
                // Thunar不支持直接选中文件，只打开文件夹
                else if (manager == "thunar") args << QFileInfo(rowData.FileName).absolutePath();
                else if (manager == "kfmclient")  args << "exec" << rowData.FileName;
                else args << QFileInfo(rowData.FileName).absolutePath();

                if (QProcess::startDetached(manager, args)) return;
            }
        }
        // 如果所有文件管理器都失败，使用xdg-open
        QProcess::startDetached("xdg-open", QStringList{} << QDir::toNativeSeparators(rowData.FileName));
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(rowData.FileName));
#endif
        break;
    }
    case IconRole::RemoveFile: removeRowData(index.row()); break;
    default: break;
    }
}

void PWTableViewModel::onSortByFileNameActionTriggered(bool descending)
{
    auto compareFunc = [descending](const RowData& a, const RowData& b) -> bool {
        if (descending) {
            return a.FileName.compare(b.FileName, Qt::CaseInsensitive) > 0;
        }
        else {
            return a.FileName.compare(b.FileName, Qt::CaseInsensitive) < 0;
        }
        };

    _performSort(compareFunc);
}

void PWTableViewModel::onSortByFileSizeActionTriggered(bool descending)
{
    auto compareFunc = [descending](const RowData& a, const RowData& b) -> bool {
        qint64 sizeA = QFileInfo(a.FileName).size();
        qint64 sizeB = QFileInfo(b.FileName).size();

        if (descending) {
            return sizeA > sizeB;
        }
        else {
            return sizeA < sizeB;
        }
        };

    _performSort(compareFunc);
}

void PWTableViewModel::onSortByFileBirthTimeActionTriggered(bool descending)
{
    auto compareFunc = [descending](const RowData& a, const RowData& b) -> bool {
        QDateTime birthTimeA = QFileInfo(a.FileName).birthTime();
        QDateTime birthTimeB = QFileInfo(b.FileName).birthTime();

        // 使用毫秒时间戳进行比较
        qint64 timeA = birthTimeA.toMSecsSinceEpoch();
        qint64 timeB = birthTimeB.toMSecsSinceEpoch();

        if (descending) {
            return timeA > timeB;
        }
        else {
            return timeA < timeB;
        }
        };

    _performSort(compareFunc);
}

void PWTableViewModel::onSortByRangeOrSizeActionTriggered(bool descending)
{
    auto compareFunc = [descending](const RowData& a, const RowData& b) -> bool {
        auto getValue = [](const RowData& data) -> int {
            if (data.isRange()) {
                return std::get<RangeData>(data.SecondColumn).Max; // 使用总页数
            }
            else if (data.isSize()) {
                const auto& size = std::get<ImageSizeData>(data.SecondColumn).OriginalSize;
                return size.width() * size.height(); // 使用像素面积
            }
            return 0;
            };

        int valueA = getValue(a);
        int valueB = getValue(b);

        if (descending) {
            return valueA > valueB;
        }
        else {
            return valueA < valueB;
        }
        };

    _performSort(compareFunc);
}

void PWTableViewModel::onSwapRows(int row1, int row2)
{
    if (row1 < 0 || row1 >= _pRowDataList.size() || row2 < 0 || row2 >= _pRowDataList.size()) return;
    std::swap(_pRowDataList[row1], _pRowDataList[row2]);
    if (_isEightColumnTable()) {
        QModelIndex newIndex2 = createIndex(row1, 4);
        _pRowDataList[row2].SwitchWidget.value()->setIndex(newIndex2);
        _pTableView->setIndexWidget(newIndex2, _pRowDataList[row2].SwitchWidget.value());

        QModelIndex newIndex1 = createIndex(row2, 4);
        _pRowDataList[row1].SwitchWidget.value()->setIndex(newIndex1);
        _pTableView->setIndexWidget(newIndex1, _pRowDataList[row1].SwitchWidget.value());
    }
    if (!property("IsGridViewMode").toBool()) {
        QModelIndex newIndex1 = createIndex(row1, 3);
        _pRowDataList[row2].RangeWidget->setIndex(newIndex1);
        _pTableView->setIndexWidget(newIndex1, _pRowDataList[row2].RangeWidget);

        QModelIndex newIndex2 = createIndex(row2, 3);
        _pRowDataList[row1].RangeWidget->setIndex(newIndex2);
        _pTableView->setIndexWidget(newIndex2, _pRowDataList[row1].RangeWidget);
    }

    QModelIndex topLeft = createIndex(row1, 0);
    QModelIndex bottomRight = createIndex(row2, columnCount() - 1);
    Q_EMIT dataChanged(topLeft, bottomRight);
}

QVariant PWTableViewModel::_formatRowData(const RowData& rowData, int column) const
{
    const QString& headerText = _pHeaderTextList.at(column);
    if (headerText == "原大小" || headerText == "压缩后大小") {
        return QLocale().formattedDataSize(QFileInfo{ rowData.FileName }.size(), 2, QLocale::DataSizeSIFormat);
    }
    switch (column) {
    case 1: { // 文件名
        if (rowData.FileName.isEmpty()) return QVariant();
        QFileInfo fileInfo(rowData.FileName);
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
            const_cast<PWTableViewModel*>(this)->_pRowDataList[rowData.Index].FileProcessState.State = FileStateType::DELETED;
            return rowData.FileName;
        }
    }
    case 2: {// 总页数 or 图片尺寸
        if (rowData.isRange()) {
            return std::get<RangeData>(rowData.SecondColumn).Max;
        }
        else if (rowData.isSize()) {
            auto size = std::get<ImageSizeData>(rowData.SecondColumn).OriginalSize;
            return QString("%1 x %2").arg(size.width()).arg(size.height());
        }
        return QVariant();
    }
    case 4: // 7列表格的状态列
    case 5: { // 8列表格的状态列
        if (!_isEightColumnTable() && column == 5) break; // 7列表格第5列不是状态列
        if (_isEightColumnTable() && column == 4) break;  // 8列表格第4列是SwitchWidget

        switch (static_cast<FileStateType>(rowData.FileProcessState.State.toInt())) {
        case FileStateType::LOADING: return "加载中";
        case FileStateType::BEREADY: return rowData.FileProcessState.Text; // 由JSON配置设置
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
    return QVariant();
}

void PWTableViewModel::_updataCellIndexWidget(int row, int column)
{   
    if (row < 0 || row >= _pRowDataList.size()) return;

    RowData& rowData = _pRowDataList[row];
    NXModelIndexWidget* widget = nullptr;

    if (column == 3) {
        widget = rowData.RangeWidget;
    }
    else if (column == 4 && _isEightColumnTable() && rowData.SwitchWidget.has_value()) {
        widget = rowData.SwitchWidget.value();
    }

    if (widget) {
        QModelIndex newIndex = createIndex(row, column);
        widget->setIndex(newIndex);
        _pTableView->setIndexWidget(newIndex, widget);
    }
}

void PWTableViewModel::_updateIRActionRowDataFrom(int startRow)
{
    for (int row = startRow; row < _pRowDataList.size(); ++row) {
        _pRowDataList[row].Index = row;
        if (_pRowDataList[row].Checked) ++_pCheckedRowCount;
        _updataCellIndexWidget(row, 3);
        if (_isEightColumnTable()) {
            _updataCellIndexWidget(row, 4);
        }
    }
}

void PWTableViewModel::_updateIRActionAllRowData()
{
    for (int row = 0; row < _pRowDataList.size(); ++row) {
        _pRowDataList[row].Index = row;
        _updataCellIndexWidget(row, 3);
        if (_isEightColumnTable()) {
            _updataCellIndexWidget(row, 4);
        }
    }
}

void PWTableViewModel::_updateResetActionAllRowData()
{
    _pCheckedRowCount = 0;
    for (int row = 0; row < _pRowDataList.size(); ++row) {
        RowData& rowData = _pRowDataList[row];
        rowData.Index = row;
        if(rowData.Checked) ++_pCheckedRowCount;
        NXModelIndexWidget* rangeWidget = _pTableView->createLineEditRangeWidget();
        if (!rangeWidget) continue;

        QList<NXLineEdit*> lineEdits = rangeWidget->findChildren<NXLineEdit*>("NXLineEdit");
        if (lineEdits.size() == 1) {
            QSignalBlocker blocker(lineEdits.front());
            QString text;
            const auto& rangeList = std::get<RangeData>(rowData.SecondColumn).RangeList;
            text.reserve(rangeList.size() * 3);
            for (const auto& range : rangeList) {
                if (range.Start == range.End) text.append(QString::number(range.Start));
                else text.append(QString("%1-%2").arg(range.Start).arg(range.End));
                text.append(QChar(0x3001));
            }
            text.chop(1);
            lineEdits.front()->setText(text);
        }
        else {
            if (rowData.isRange()) {
                const auto& range = std::get<RangeData>(rowData.SecondColumn);
                QSignalBlocker blocker1(lineEdits.front());
                QSignalBlocker blocker2(lineEdits.back());
                lineEdits.front()->setText(QString::number(range.RangeList.front().Start));
                lineEdits.back()->setText(QString::number(range.RangeList.front().End));
            }
            else {
                QSignalBlocker blocker1(lineEdits[0]);
                QSignalBlocker blocker2(lineEdits[1]);
                const auto& size = std::get<ImageSizeData>(rowData.SecondColumn);
                lineEdits.front()->setText(QString::number(size.ResizedSize.width()));
                lineEdits.back()->setText(QString::number(size.ResizedSize.height()));
            }
        }
        QModelIndex rangeIndex = createIndex(row, 3);
        _pTableView->setIndexWidget(rangeIndex, rangeWidget);
        if (_isEightColumnTable()) {
            NXModelIndexWidget* switchWidget = _pTableView->createSwitchWidget();
            QModelIndex switchIndex = createIndex(row, 4);
            _pTableView->setIndexWidget(switchIndex, switchWidget);
        }
    }
}

void PWTableViewModel::_updateMoveActionAllRowData(int fromRow, int toRow)
{
    for (int row = fromRow; row <= toRow; ++row) {
        RowData& rowData = _pRowDataList[row];
        rowData.Index = row;
        NXModelIndexWidget* rangeWidget = _pTableView->createLineEditRangeWidget();
        if (!rangeWidget) continue;

        QList<NXLineEdit*> lineEdits = rangeWidget->findChildren<NXLineEdit*>("NXLineEdit");
        if (lineEdits.size() == 1) {
            QSignalBlocker blocker(lineEdits.front());
            QString text;
            const auto& rangeList = std::get<RangeData>(rowData.SecondColumn).RangeList;
            text.reserve(rangeList.size() * 3);
            for (const auto& range : rangeList) {
                if(range.Start == range.End) text.append(QString::number(range.Start));
                else text.append(QString("%1-%2").arg(range.Start).arg(range.End));
                text.append(QChar(0x3001));
            }
            text.chop(1);
            lineEdits.front()->setText(text);
            lineEdits.front()->setProperty("FullRangeText", text);
        }
        else {
            if (rowData.isRange()) {
                const auto& range = std::get<RangeData>(rowData.SecondColumn);
                QSignalBlocker blocker1(lineEdits.front());
                QSignalBlocker blocker2(lineEdits.back());
                lineEdits.front()->setText(QString::number(range.RangeList.front().Start));
                lineEdits.back()->setText(QString::number(range.RangeList.front().End));
            }
            else {
                QSignalBlocker blocker1(lineEdits[0]);
                QSignalBlocker blocker2(lineEdits[1]);
                const auto& size = std::get<ImageSizeData>(rowData.SecondColumn);
                lineEdits.front()->setText(QString::number(size.ResizedSize.width()));
                lineEdits.back()->setText(QString::number(size.ResizedSize.height()));
            }
        }
        QModelIndex rangeIndex = createIndex(row, 3);
        _pTableView->setIndexWidget(rangeIndex, rangeWidget);
        if (_isEightColumnTable()) {
            NXModelIndexWidget* switchWidget = _pTableView->createSwitchWidget();
            QModelIndex switchIndex = createIndex(row, 4);
            _pTableView->setIndexWidget(switchIndex, switchWidget);
        }
    }
}

void PWTableViewModel::_setupWidgetForNewRow(int row, RowData& rowData)
{
    auto setupWidgetFunc = [&](int column, QPointer<NXModelIndexWidget> widget) {
        if (widget) {
            QModelIndex newIndex = createIndex(row, column);
            widget->setIndex(newIndex);
            _pTableView->setIndexWidget(newIndex, widget);
        }
        };

    setupWidgetFunc(3, rowData.RangeWidget);
    if (_isEightColumnTable() && rowData.SwitchWidget.has_value()) {
        setupWidgetFunc(4, rowData.SwitchWidget.value());
    }
}

void PWTableViewModel::_performSort(const std::function<bool(const RowData&, const RowData&)>& compareFunc)
{
    if (_pRowDataList.isEmpty() || _pRowDataList.size() == 1) return;

    beginResetModel();
    std::sort(_pRowDataList.begin(), _pRowDataList.end(), compareFunc);
    endResetModel();

    _updateResetActionAllRowData();
    Q_EMIT headerDataChanged(Qt::Horizontal, 0, 1);
}

bool PWTableViewModel::_isEightColumnTable() const
{
    return _pTableView->getModuleType().MasterModuleType == WizConverter::Enums::MasterModule::Type::PDFToWord;
}