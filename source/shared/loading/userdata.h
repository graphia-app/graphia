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

#ifndef USERDATA_H
#define USERDATA_H

#include "shared/loading/iuserdata.h"
#include "shared/loading/userdatavector.h"

#include "shared/utils/pair_iterator.h"
#include "shared/utils/progressable.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSet>

#include <json_helper.h>

#include <map>
#include <vector>

class UserData : public QObject, public virtual IUserData
{
    Q_OBJECT

private:
    std::map<QString, UserDataVector> _userDataVectors;
    std::vector<QString> _vectorNames;
    size_t _numValues = 0;

public:
    QString firstVectorName() const override;

    size_t numUserDataVectors() const override;
    size_t numValues() const override;

    bool empty() const { return _userDataVectors.empty(); }

    const std::vector<QString>& vectorNames() const override;

    auto begin() const { return _vectorNames.begin(); }
    auto end() const { return _vectorNames.end(); }

    void add(const QString& name) override;
    bool setValue(size_t index, const QString& name, const QString& value) override;
    QVariant value(size_t index, const QString& name) const override;

    UserDataVector* vector(const QString& name);
    void setVector(UserDataVector&& other);

    virtual void remove(const QString& name);

    json save(Progressable& progressable, const std::vector<size_t>& indexes = {}) const;
    bool load(const json& jsonObject, Progressable& progressable);

signals:
    void vectorValuesChanged(const QString& vectorName);
};

#endif // USERDATA_H
