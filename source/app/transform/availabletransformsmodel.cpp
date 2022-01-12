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

#include "availabletransformsmodel.h"

#include "shared/utils/static_block.h"

#include "application.h"
#include "graph/graphmodel.h"

#include "transform/graphtransform.h"

AvailableTransformsModel::AvailableTransformsModel(const GraphModel& graphModel,
                                                   QObject* parent) :
    QAbstractListModel(parent),
    _graphModel(&graphModel),
    _transformNames(graphModel.availableTransformNames())
{}

QVariant AvailableTransformsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() >= _transformNames.size())
        return {};

    auto transformName = _transformNames.at(index.row());

    if(role != Qt::DisplayRole)
    {
        const auto* transform = _graphModel->transformFactory(transformName);

        if(transform == nullptr)
            return {};

        switch(role)
        {
        case Roles::TransformCategoryRole:
        {
            if(transform->category().isEmpty())
                return tr("Uncategorised");

            return transform->category();
        }
        default:
            return {};
        }
    }

    return transformName;
}

int AvailableTransformsModel::rowCount(const QModelIndex& /*parentIndex*/) const
{
    return _transformNames.size();
}

QVariant AvailableTransformsModel::get(const QModelIndex& index) const
{
    return data(index, Qt::DisplayRole).toString();
}

QHash<int, QByteArray> AvailableTransformsModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::TransformCategoryRole] = "category";

    return names;
}

static_block
{
    qmlRegisterInterface<AvailableTransformsModel>("AvailableTransformsModel"
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        , Application::majorVersion()
#endif
        );
}
