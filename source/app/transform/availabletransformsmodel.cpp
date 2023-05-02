/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
#include "app/preferences.h"

#include "transform/graphtransform.h"

#include <json_helper.h>

using namespace Qt::Literals::StringLiterals;

AvailableTransformsModel::AvailableTransformsModel(const GraphModel& graphModel,
                                                   QObject* parent) :
    QAbstractListModel(parent),
    _graphModel(&graphModel),
    _transformNames(graphModel.availableTransformNames())
{
    auto updateFavouriteTransforms = [this](const QString& favouriteTransformsString)
    {
        auto jsonArray = parseJsonFrom(favouriteTransformsString.toUtf8());

        if(jsonArray.is_null() || !jsonArray.is_array())
            return;

        QStringList favouriteTransforms;
        favouriteTransforms.reserve(static_cast<int>(jsonArray.size()));

        for(const auto& transformName : jsonArray)
        {
            if(transformName.is_string())
                favouriteTransforms.append(QString::fromStdString(transformName));
        }

        beginResetModel();
        _favouriteTransforms = favouriteTransforms;
        endResetModel();
    };

    updateFavouriteTransforms(u::pref(u"misc/favouriteTransforms"_s).toString());

    connect(&_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
    [=](const QString& key, const QVariant& value)
    {
        if(key == u"misc/favouriteTransforms"_s)
            updateFavouriteTransforms(value.toString());
    });
}

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
            if(u::contains(_favouriteTransforms, transformName))
                return tr("Favourites");

            // Any invalid category names are counted as "uncategorised"
            if(transform->category().isEmpty() || transform->category() == tr("Favourites"))
                return tr("Uncategorised");

            return transform->category();
        }

        case Roles::TransformFavouriteRole:
            return u::contains(_favouriteTransforms, transformName);

        default:
            return {};
        }
    }

    return transformName;
}

int AvailableTransformsModel::rowCount(const QModelIndex& /*parentIndex*/) const
{
    return static_cast<int>(_transformNames.size());
}

QVariant AvailableTransformsModel::get(const QModelIndex& index) const
{
    return data(index, Qt::DisplayRole).toString();
}

QHash<int, QByteArray> AvailableTransformsModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();

    names[Roles::TransformCategoryRole] = "category";
    names[Roles::TransformFavouriteRole] = "isFavourite";

    return names;
}

Q_DECLARE_INTERFACE(AvailableTransformsModel, APP_URI)

static_block
{
    qmlRegisterInterface<AvailableTransformsModel>("AvailableTransformsModel", Application::majorVersion());
}
