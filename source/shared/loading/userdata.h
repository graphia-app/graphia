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

#ifndef USERDATA_H
#define USERDATA_H

#include "userdatavector.h"

#include "shared/utils/pair_iterator.h"
#include "shared/utils/progressable.h"

#include <QString>
#include <QVariant>
#include <QSet>

#include <json_helper.h>

#include <vector>

class UserData
{
private:
    // This is not a map because the data needs to be ordered
    std::vector<std::pair<QString, UserDataVector>> _userDataVectors;
    std::vector<QString> _vectorNames;
    int _numValues = 0;

public:
    virtual ~UserData() = default;

    QString firstUserDataVectorName() const;

    int numUserDataVectors() const;
    int numValues() const;

    bool empty() const { return _userDataVectors.empty(); }

    std::vector<QString> vectorNames() const;

    auto begin() const { return _userDataVectors.begin(); }
    auto end() const { return _userDataVectors.end(); }

    UserDataVector& add(QString name);
    void setValue(size_t index, const QString& name, const QString& value);
    QVariant value(size_t index, const QString& name) const;

    UserDataVector* vector(const QString& name);
    void setVector(const QString& name, UserDataVector&& other);

    virtual void remove(const QString& name);

    json save(Progressable& progressable, const std::vector<size_t>& indexes = {}) const;
    bool load(const json& jsonObject, Progressable& progressable);
};

#endif // USERDATA_H
