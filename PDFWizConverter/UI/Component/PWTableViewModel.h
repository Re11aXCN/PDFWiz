#pragma once

#include <QAbstractTableModel>
#include "PWDef.h"

class PWTableViewModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PRIVATE_CREATE_D(int, CheckedRowCount)
public:
    struct RowData {
        QVariantList CellData;
        int Index{0};
        FileStateType State{ FileStateType::LOADING };
        bool Checked{ false };
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

    void setCellData(int row, const QVariantList& cellDatas);
    void setCellData(int row, QVariantList&& cellDatas);
    void setCellData(int row, int column, const QVariant& value);
    QVariant getCellData(int row, int column) const;

    void setRowData(const QList<RowData>& rowDataList);
    void setRowData(QList<RowData>&& rowDataList);
    RowData& getRowData(int row);
    void appendRowData(const RowData& rowData);
    void appendRowData(RowData&& rowData);

    void insertRowData(int row, const RowData& rowData);
    void insertRowData(int row, RowData&& rowData);

    void removeRowData(int row);
    void removeRowData(const QModelIndex& index);
public Q_SLOTS:
    void onSelectSingleRow(const QModelIndex& index);
    void onSelectAllRows();
    void onRemoveSingleRow(const QModelIndex& index);
    void onRemoveSelectedRows();
    void onRemoveNotSelectedRows();
    void onRemoveAllRows();
private:
    QVariant _formatRowData(const RowData& rowData, int column) const;
    void _updateRowIndexesFrom(int startRow);
    void _updateAllRowIndexes();
    void _updateAllRowStates();

    QStringList _pHeaderTextList;
    QList<RowData> _pRowDataList;
    QList<QIcon> _pCheckIconList;
    QList<QIcon> _pFileTypeIconList;
};