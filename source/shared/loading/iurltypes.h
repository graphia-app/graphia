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

#ifndef IURLTYPES_H
#define IURLTYPES_H

#include <QStringList>
#include <QString>

class IUrlTypes
{
public:
    virtual ~IUrlTypes() = default;

    virtual QStringList loadableUrlTypeNames() const = 0;
    virtual QString individualDescriptionForUrlTypeName(const QString& urlTypeName) const = 0;
    virtual QString collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const = 0;
    virtual QStringList extensionsForUrlTypeName(const QString& urlTypeName) const = 0;
};

#endif // IURLTYPES_H
