#pragma once

#include <QAbstractTableModel>
#include <PWDef.h>

class PWTableViewModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit PWTableViewModel(QObject* parent = nullptr);
    ~PWTableViewModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value,
        int role = Qt::EditRole) override;

private:
    QStringList _pHeader;
    QList<QStringList> _pDataList;
    QList<QIcon> _pCheckIconList;
    QList<QIcon> _pFileTypeIconList;
};