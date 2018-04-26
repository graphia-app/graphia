#ifndef DATARECTTABLEMODEL_H
#define DATARECTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "shared/loading/tabulardata.h"

class DataRectTableModel : public QAbstractTableModel
{
private:
    TabularData _data;
public:
    DataRectTableModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    void setTabularData(TabularData data);
    TabularData* tabularData();
};

#endif // DATARECTTABLEMODEL_H
