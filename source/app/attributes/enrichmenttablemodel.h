#ifndef ENRICHMENTTABLEMODEL_H
#define ENRICHMENTTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include <json_helper.h>

class EnrichmentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Results
    {
        SelectionA,
        SelectionB,
        Observed,
        ExpectedTrial,
        OverRep,
        Fishers,
        AdjustedFishers,
        NumResultColumns
    };
    Q_ENUM(Results)

    explicit EnrichmentTableModel(QObject* parent = nullptr);
    using Row = std::vector<QVariant>;
    using Table = std::vector<Row>;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant data(int row, const QString& role);
    int rowFromAttributeSets(const QString& attributeA, const QString& attributeB);
    QHash<int, QByteArray> roleNames() const override;

    void setTableData(Table data);
    Q_INVOKABLE QString resultToString(EnrichmentTableModel::Results result);
    Q_INVOKABLE bool resultIsNumerical(EnrichmentTableModel::Results result);

    json toJson();
private:
    Table _data;
};

#endif // ENRICHMENTTABLEMODEL_H
