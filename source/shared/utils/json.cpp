/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "json.h"

#include "shared/loading/iparser.h"
#include "shared/graph/elementid.h"
#include "shared/utils/progressable.h"
#include "shared/loading/progress_iterator.h"

#include <QString>
#include <QUrl>
#include <QFile>
#include <QByteArray>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <vector>

#include <json.hpp>
using json = nlohmann::json;

void to_json(json &j, const QString &s)
{
    j = s.toStdString();
}

void to_json(json &j, NodeId nodeId)
{
    j = static_cast<int>(nodeId);
}

void to_json(json &j, EdgeId edgeId)
{
    j = static_cast<int>(edgeId);
}

void to_json(json &j, ComponentId componentId)
{
    j = static_cast<int>(componentId);
}

void from_json(const json &j, QString &s)
{
    s = QString::fromStdString(j.get<std::string>());
}

void from_json(const json &j, QUrl &url)
{
    url = QString::fromStdString(j.get<std::string>());
}

void from_json(const json &j, QVariant &v)
{
    switch(j.type())
    {
    default:
    case json::value_t::null:               v = {}; break;
    case json::value_t::boolean:            v = QVariant::fromValue(j.get<bool>()); break;
    case json::value_t::number_integer:     v = QVariant::fromValue(j.get<qint64>()); break;
    case json::value_t::number_unsigned:    v = QVariant::fromValue(j.get<quint64>()); break;
    case json::value_t::number_float:       v = QVariant::fromValue(j.get<double>()); break;
    case json::value_t::string:             v = QVariant::fromValue(QString::fromStdString(j.get<std::string>())); break;
    case json::value_t::array:
    {
        QVariantList variantList;
        variantList.reserve(static_cast<int>(j.size()));

        for(const auto& element : j)
        {
            QVariant variantElement;
            from_json(element, variantElement);
            variantList.append(variantElement);
        }

        v = variantList;
        break;
    }
    case json::value_t::object:
    {
        QVariantMap variantMap;

        for(const auto& element : j.items())
        {
            auto key = QString::fromStdString(element.key());
            QVariant variantValue;
            from_json(element.value(), variantValue);
            variantMap.insert(key, variantValue);
        }

        v = QVariant::fromValue(variantMap);
        break;
    }
    }
}

json jsonArrayFrom(const std::vector<QString> &container, Progressable *progressable)
{
    json array;

    uint64_t i = 0;
    for(const auto& value : container)
    {
        array.emplace_back(value);

        if(progressable != nullptr)
            progressable->setProgress(static_cast<int>((i++) * 100 / container.size()));
    }

    if(progressable != nullptr)
        progressable->setProgress(-1);

    return array;
}

json parseJsonFrom(const QByteArray &byteArray, IParser *parser)
{
    json result;

    using JSONByteArrayIterator = progress_iterator<QByteArray::const_iterator>;
    JSONByteArrayIterator it(byteArray.begin());
    const JSONByteArrayIterator end(byteArray.end());

    if(parser != nullptr)
    {
        it.onPositionChanged(
            [&byteArray, &parser](size_t position)
            {
                parser->setProgress(static_cast<int>((position * 100) /
                    static_cast<size_t>(byteArray.size())));
            });

        it.setCancelledFn([&parser] { return parser->cancelled(); });
    }

    try
    {
#ifdef DISABLE_EXCEPTION_THROWING
        result = json::parse(it, end, nullptr, false);
#else
        result = json::parse(it, end);
#endif
    }
    catch(json::parse_error& e)
    {
        if(parser != nullptr)
            parser->setFailureReason(QString::fromStdString(e.what()));
    }

    return result;
}

json parseJsonFrom(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODeviceBase::ReadOnly))
        return {};

    return parseJsonFrom(file.readAll());
}