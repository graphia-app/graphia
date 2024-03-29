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

#ifndef USERDATAVECTOR_H
#define USERDATAVECTOR_H

#include <QString>

#include "shared/utils/typeidentity.h"

#include <json_helper.h>

#include <vector>
#include <limits>
#include <utility>

#include <QStringList>

class UserDataVector : public TypeIdentity
{
private:
    QString _name;

    std::vector<QString> _values;

public:
    UserDataVector() = default;
    UserDataVector(const UserDataVector&) = default;
    UserDataVector(UserDataVector&&) noexcept = default;

    UserDataVector& operator=(const UserDataVector&) = default;
    UserDataVector& operator=(UserDataVector&&) = default;

    explicit UserDataVector(const QString& name) :
        _name(name)
    {}

    auto begin() const { return _values.begin(); }
    auto end() const { return _values.end(); }

    QStringList toStringList() const;

    const QString& name() const { return _name; }
    size_t numValues() const { return _values.size(); }
    size_t numUniqueValues() const;
    void reserve(size_t size) { _values.reserve(size); }

    bool set(size_t index, const QString& value);
    QString get(size_t index) const;

    bool resetType();

    json save(const std::vector<size_t>& indexes = {}) const;
    bool load(const QString& name, const json& jsonObject);
};

#endif // USERDATAVECTOR_H
