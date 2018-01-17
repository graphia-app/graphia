#ifndef ENRICHMENTTABLEMODEL_H
#define ENRICHMENTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class EnrichmentTableModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    using Row = std::vector<QVariant>;
    using Table = std::vector<Row>;

    Table _data;
public:
    EnrichmentTableModel();

    const int COLUMN_COUNT = 9;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTableData(Table data);
private slots:
    void updateComplete();


};

#endif // ENRICHMENTTABLEMODEL_H
