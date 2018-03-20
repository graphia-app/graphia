#ifndef ENRICHMENTTABLEMODEL_H
#define ENRICHMENTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "json_helper.h"

class EnrichmentTableModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    using Row = std::vector<QVariant>;
    using Table = std::vector<Row>;

    Table _data;
public:
    EnrichmentTableModel(QObject* parent = nullptr);

    const int COLUMN_COUNT = 7;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant data(int row, QString role);
    int rowFromAttributeSets(QString attributeA, QString attributeB);
    QHash<int, QByteArray> roleNames() const override;

    void setTableData(Table data);

    json toJson();
};

#endif // ENRICHMENTTABLEMODEL_H
