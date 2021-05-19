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

#ifndef IUSERDATA_H
#define IUSERDATA_H

#include <QString>
#include <QVariant>

#include <vector>

class IUserData
{
public:
    virtual ~IUserData() = default;

    virtual QString firstVectorName() const = 0;

    virtual int numUserDataVectors() const = 0;
    virtual int numValues() const = 0;

    virtual const std::vector<QString>& vectorNames() const = 0;

    virtual void add(const QString& name) = 0;

    virtual QVariant value(size_t index, const QString& name) const = 0;
    virtual bool setValue(size_t index, const QString& name, const QString& value) = 0;
};

#endif // IUSERDATA_H
