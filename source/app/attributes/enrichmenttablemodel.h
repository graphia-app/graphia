/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

class EnrichmentTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectionA READ selectionA WRITE setSelectionA NOTIFY selectionNamesChanged)
    Q_PROPERTY(QString selectionB READ selectionB WRITE setSelectionB NOTIFY selectionNamesChanged)

public:
    enum Results
    {
        SelectionA,
        SelectionB,
        Observed,
        ExpectedTrial,
        OverRep,
        Fishers,
        BonferroniAdjusted,
        NumResultColumns
    };
    Q_ENUM(Results)

    enum Roles
    {
        EnrichedRole = Qt::UserRole + 1
    };

    explicit EnrichmentTableModel(QObject* parent = nullptr);
    using Row = std::vector<QVariant>;
    using Table = std::vector<Row>;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant data(int row, EnrichmentTableModel::Results result) const;
    int rowFromAttributeSets(const QString& attributeA, const QString& attributeB);
    QHash<int, QByteArray> roleNames() const override;

    QString selectionA() const { return _selectionA; }
    void setSelectionA(const QString& selectionA);
    QString selectionB() const { return _selectionB; }
    void setSelectionB(const QString& selectionB);

    void setTableData(Table data, QString selectionA, QString selectionB);

private:
    Table _data;
    QString _selectionA;
    QString _selectionB;

    QHash<int, QByteArray> _roleNames;

signals:
    void selectionNamesChanged();
};

#endif // ENRICHMENTTABLEMODEL_H
