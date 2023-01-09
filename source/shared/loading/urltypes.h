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

#ifndef URLTYPES_H
#define URLTYPES_H

#include "iurltypes.h"

#include <QString>
#include <QStringList>

#include <map>
#include <utility>

class QUrl;

class UrlTypes : public virtual IUrlTypes
{
private:
    class UrlType
    {
    public:
        UrlType(const QString& individualDescription,
                const QString& collectiveDescription,
                const QStringList& extensions) :
            _individualDescription(individualDescription),
            _collectiveDescription(collectiveDescription),
            _extensions(extensions)
        {}

        const QString& individualDescription() const { return _individualDescription; }
        const QString& collectiveDescription() const { return _collectiveDescription; }
        QStringList extensions() const { return _extensions; }

    private:
        QString _individualDescription;
        QString _collectiveDescription;
        QStringList _extensions;
    };

    std::map<QString, UrlType> _urlTypes;

protected:
    void registerUrlType(const QString& urlTypeName,
                         const QString& individualDescription,
                         const QString& collectiveDescription,
                         const QStringList& extensions);

    QStringList identifyByExtension(const QUrl& url) const;

public:
    ~UrlTypes() override = default;

    QStringList loadableUrlTypeNames() const override;
    QString individualDescriptionForUrlTypeName(const QString& urlTypeName) const override;
    QString collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const override;
    QStringList extensionsForUrlTypeName(const QString& urlTypeName) const override;
};

#endif // URLTYPES_H
