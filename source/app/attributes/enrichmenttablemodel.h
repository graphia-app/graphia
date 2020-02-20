/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    QVariant data(int row, EnrichmentTableModel::Results result) const;
    int rowFromAttributeSets(const QString& attributeA, const QString& attributeB);
    QHash<int, QByteArray> roleNames() const override { return _roleNames; }

    void setTableData(Table data);
    Q_INVOKABLE QString resultToString(EnrichmentTableModel::Results result);
    Q_INVOKABLE bool resultIsNumerical(EnrichmentTableModel::Results result);

    json toJson();

private:
    Table _data;
    QHash<int, QByteArray> _roleNames;
};

#endif // ENRICHMENTTABLEMODEL_H
