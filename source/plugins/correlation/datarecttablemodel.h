#ifndef DATARECTTABLEMODEL_H
#define DATARECTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "shared/loading/tabulardata.h"

class DataRectTableModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int MAX_COLUMNS MEMBER MAX_COLUMNS CONSTANT)
private:
    int MAX_COLUMNS = 200;
    TabularData* _data = nullptr;
    bool _transposed = false;
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setTabularData(TabularData& data);
    TabularData* tabularData();

    bool transposed() const;
    void setTransposed(bool transposed);
};

#endif // DATARECTTABLEMODEL_H
