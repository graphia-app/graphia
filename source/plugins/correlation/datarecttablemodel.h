#ifndef DATARECTTABLEMODEL_H
#define DATARECTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "shared/loading/tabulardata.h"

class DataRectTableModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    TabularData _data;
public:
    DataRectTableModel() = default;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setTabularData(TabularData data);
    TabularData* tabularData();
};

#endif // DATARECTTABLEMODEL_H
