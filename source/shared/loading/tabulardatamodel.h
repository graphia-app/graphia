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

#ifndef TABULARDATAMODEL_H
#define TABULARDATAMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "shared/loading/tabulardata.h"

class TabularDataModel : public QAbstractTableModel
{
    Q_OBJECT

private:
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

    Q_INVOKABLE bool columnIsNumerical(int column) const;
};

#endif // TABULARDATAMODEL_H
