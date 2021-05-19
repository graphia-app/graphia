/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef AVAILABLETRANSFORMSMODEL_H
#define AVAILABLETRANSFORMSMODEL_H

#include <QStringList>
#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

class GraphModel;

class AvailableTransformsModel : public QAbstractListModel
{
    Q_OBJECT

private:
    enum Roles
    {
        TransformCategoryRole = Qt::UserRole + 1
    };

    const GraphModel* _graphModel = nullptr;
    QStringList _transformNames;

public:
    AvailableTransformsModel() = default;
    explicit AvailableTransformsModel(const GraphModel& graphModel,
                             QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant get(const QModelIndex& index) const;

    QHash<int, QByteArray> roleNames() const override;
};

#endif // AVAILABLETRANSFORMSMODEL_H
