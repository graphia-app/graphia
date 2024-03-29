/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "urltypes.h"

#include "shared/utils/container.h"

#include <QFileInfo>
#include <QUrl>

void UrlTypes::registerUrlType(const QString& urlTypeName,
                               const QString& individualDescription,
                               const QString& collectiveDescription,
                               const QStringList& extensions)
{
    _urlTypes.emplace(urlTypeName, UrlType(individualDescription, collectiveDescription, extensions));
}

QStringList UrlTypes::identifyByExtension(const QUrl& url) const
{
    QStringList urlTypeNames;

    if(url.isLocalFile())
    {
        const QString extension = QFileInfo(url.toLocalFile()).suffix();

        for(const auto& urlType : _urlTypes)
        {
            for(auto& loadableExtension : urlType.second.extensions())
            {
                if(loadableExtension.compare(extension, Qt::CaseInsensitive) == 0)
                    urlTypeNames.append(urlType.first);
            }
        }
    }

    return urlTypeNames;
}

QStringList UrlTypes::loadableUrlTypeNames() const
{
    QStringList urlTypeNames;
    urlTypeNames.reserve(static_cast<int>(_urlTypes.size()));
    for(const auto& urlType : _urlTypes)
        urlTypeNames.append(urlType.first);

    return urlTypeNames;
}

QString UrlTypes::individualDescriptionForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).individualDescription();
}

QString UrlTypes::collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).collectiveDescription();
}

QStringList UrlTypes::extensionsForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).extensions();
}
