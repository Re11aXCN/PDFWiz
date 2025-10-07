#pragma once

#include <QAbstractTableModel>
#include <QPointer>

#include "PWDef.h"

#include "PWTableViewIconDelegate.h"
class PWTableView;
class NXModelIndexWidget;
class PWTableViewModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(int, CheckedRowCount)
public:
    struct RangeData {
        struct Range {
            int Start{ 1 };
            int End{ 1 };
        };
        QList<Range> RangeList;
        int Max{ 1 };
    };
    struct ImageSizeData {
        QSize OriginalSize{};
        QSize ResizedSize{};
    };
    struct FileProcessState {
        FileStates State{ FileStateType::LOADING };
        QString Text{ "" };
    };
    struct ExpandData {
        QString Password{};
        bool HasImage{ false };
    };

    struct RowData {
        // 基础字段 - 所有表格类型都有
        bool Checked{ false };
        int Index{ 0 };
        QString FileName;
        std::variant<RangeData, ImageSizeData> SecondColumn;
        QPointer<NXModelIndexWidget> RangeWidget{ nullptr };
        FileProcessState FileProcessState{};
        ExpandData ExpandData;
        // 可选字段 - 仅8列表格有
        std::optional<QPointer<NXModelIndexWidget>> SwitchWidget; // 第四列：SwitchWidget指针（可选）

        // 便利方法
        bool hasSwitchWidget() const { return SwitchWidget.has_value(); }

        // 获取第二列数据的类型安全方法
        bool isRange() const { return std::holds_alternative<RangeData>(SecondColumn); }
        bool isSize() const { return std::holds_alternative<ImageSizeData>(SecondColumn); }

        std::optional<RangeData> getRangeData() const {
            if (isRange()) return std::get<RangeData>(SecondColumn);
            return std::nullopt;
        }

        std::optional<ImageSizeData> getImageSizeData() const {
            if (isSize()) return std::get<ImageSizeData>(SecondColumn);
            return std::nullopt;
        }

        // 设置第二列数据的类型安全方法
        void setRangeData(const RangeData& range) { SecondColumn = range; }
        void setImageSizeData(const ImageSizeData& size) { SecondColumn = size; }
    };
    explicit PWTableViewModel(QObject* parent = nullptr);
    ~PWTableViewModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value,
        int role = Qt::EditRole) override;

    void setHeaderTextList(const QStringList& headerTextList);

    void setRowData(const QList<RowData>& rowDataList);
    void setRowData(QList<RowData>&& rowDataList);
    RowData& getRowData(int row);
    const RowData& getRowData(int row) const;

    void appendRowData(const RowData& rowData);
    void appendRowData(RowData&& rowData);
    void insertRowData(int row, const RowData& rowData);
    void insertRowData(int row, RowData&& rowData);
    void removeRowData(int row);
    void removeRowData(const QModelIndex& index);

    void appendRowData(const QList<RowData>& rowDataList);
    void appendRowData(QList<RowData>&& rowDataList);
    void insertRowData(int row, const QList<RowData>& rowDataList);
    void insertRowData(int row, QList<RowData>&& rowDataList);
    void removeRowData(QList<int>&& rows);
    void removeRowData(const QModelIndexList& indexes);

    // 获取选中行的数据
    QList<RowData> getSelectedRows() const;
    QList<int> getSelectedRowIndexes() const;

    // 更新行状态
    void updateRowState(int row, FileStateType state, const QString& text = "");
    void updateRowRangeData(int row, const RangeData& range);
    void updateRowSize(int row, const ImageSizeData& size);
public Q_SLOTS:
    void onSelectSingleRow(const QModelIndex& index);
    void onSelectAllRows();
    void onRemoveSingleRow(const QModelIndex& index);
    void onRemoveSelectedRows();
    void onRemoveNotSelectedRows();
    void onRemoveAllRows();
    void onDelegateIconClicked(PWTableViewIconDelegate::IconRole role, const QModelIndex& index);
    void onSortByFileNameActionTriggered(bool descending);
    void onSortByFileSizeActionTriggered(bool descending);
    void onSortByFileBirthTimeActionTriggered(bool descending);
    void onSortByRangeOrSizeActionTriggered(bool descending);

private:
    QVariant _formatRowData(const RowData& rowData, int column) const;
    void _updataCellIndexWidget(int row, int column);
    void _updateIRActionRowDataFrom(int startRow);
    void _updateIRActionAllRowData();
    void _updateResetActionAllRowData();

    void _setupWidgetForNewRow(int row, RowData& rowData);
    void _performSort(const std::function<bool(const RowData&, const RowData&)>& compareFunc);
    // 检查是否为8列表格
    bool _isEightColumnTable() const;

    QStringList _pHeaderTextList;
    QList<RowData> _pRowDataList;
    QList<QIcon> _pCheckIconList;
    QList<QIcon> _pFileTypeIconList;

    PWTableView* _pTableView{ nullptr };
};