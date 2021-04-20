/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::lowest();
    double _floatMin = std::numeric_limits<double>::max();
    double _floatMax = std::numeric_limits<double>::lowest();

public:
    UserDataVector() = default;
    UserDataVector(const UserDataVector&) = default;
    UserDataVector(UserDataVector&&) noexcept = default;

    UserDataVector& operator=(const UserDataVector&) = default;
    UserDataVector& operator=(UserDataVector&&) = default;

    explicit UserDataVector(QString name) :
        _name(std::move(name))
    {}

    auto begin() const { return _values.begin(); }
    auto end() const { return _values.end(); }

    QStringList toStringList() const;

    const QString& name() const { return _name; }
    int numValues() const { return static_cast<int>(_values.size()); }
    int numUniqueValues() const;
    void reserve(int size) { _values.reserve(size); }
    void clear() { _values.clear(); }

    int intMin() const { return _intMin; }
    int intMax() const { return _intMax; }
    double floatMin() const { return _floatMin; }
    double floatMax() const { return _floatMax; }

    void set(size_t index, const QString& value);
    QString get(size_t index) const;

    json save(const std::vector<size_t>& indexes = {}) const;
    bool load(const QString& name, const json& jsonObject);
};

#endif // USERDATAVECTOR_H
