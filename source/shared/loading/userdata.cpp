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

#include "userdata.h"

#include "shared/utils/container.h"

#include <QDebug>

QString UserData::firstVectorName() const
{
    if(!_vectorNames.empty())
        return _vectorNames.front();

    return {};
}

int UserData::numUserDataVectors() const
{
    return static_cast<int>(_userDataVectors.size());
}

int UserData::numValues() const
{
    return _numValues;
}

static QString normalise(QString name)
{
    if(name.isEmpty())
        return QObject::tr("Unnamed");

    // These may ultimately be used as attribute names, and
    // dots aren't allowed there
    name.replace('.', '_');

    return name;
}

const std::vector<QString>& UserData::vectorNames() const
{
    return _vectorNames;
}

void UserData::add(const QString& name)
{
    QString normalisedName = normalise(name);
    if(u::contains(_vectorNames, normalisedName))
        return;

    _vectorNames.emplace_back(normalisedName);
    _userDataVectors.emplace(normalisedName, UserDataVector(normalisedName));
}

bool UserData::setValue(size_t index, const QString& name, const QString& value)
{
    QString normalisedName = normalise(name);

    // Make sure the vector exists first
    add(normalisedName);

    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
        [&normalisedName](const auto& it2) { return it2.first == normalisedName; });

    Q_ASSERT(it != _userDataVectors.end());

    auto& userDataVector = it->second;

    bool changed = userDataVector.set(index, value);
    _numValues = std::max(_numValues, userDataVector.numValues());

    if(changed)
        emit vectorValuesChanged(name);

    return changed;
}

QVariant UserData::value(size_t index, const QString& name) const
{
    QString normalisedName = normalise(name);
    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
        [&normalisedName](const auto& it2) { return it2.first == normalisedName; });

    if(it != _userDataVectors.end())
    {
        const auto& userDataVector = it->second;
        const auto& stringValue = userDataVector.get(index);

        switch(userDataVector.type())
        {
        default:
        case UserDataVector::Type::Unknown:
        case UserDataVector::Type::String:
            return stringValue;

        case UserDataVector::Type::Float:
            return stringValue.toFloat();

        case UserDataVector::Type::Int:
            return stringValue.toInt();
        }
    }

    return {};
}

UserDataVector* UserData::vector(const QString& name)
{
    QString normalisedName = normalise(name);
    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
    [&normalisedName](const auto& pair)
    {
        return pair.first == normalisedName;
    });

    return it != _userDataVectors.end() ? &it->second : nullptr;
}

void UserData::setVector(UserDataVector&& other)
{
    QString normalisedName = normalise(other.name());
    add(normalisedName);
    *vector(normalisedName) = std::move(other);
}

void UserData::remove(const QString& name)
{
    QString normalisedName = normalise(name);
    _userDataVectors.erase(normalisedName);
    u::removeByValue(_vectorNames, normalisedName);
}

json UserData::save(Progressable& progressable, const std::vector<size_t>& indexes) const
{
    json jsonObject;

    int i = 0;

    std::vector<std::map<std::string, json>> vectors;
    for(const auto& vectorName : _vectorNames)
    {
        const auto& userDataVector = _userDataVectors.at(vectorName);

        std::map<std::string, json> vector;
        vector.emplace(vectorName.toStdString(), userDataVector.save(indexes));
        vectors.emplace_back(vector);
        progressable.setProgress((i++ * 100) / static_cast<int>(_userDataVectors.size()));
    }

    jsonObject["vectors"] = vectors;

    progressable.setProgress(-1);

    return jsonObject;
}

bool UserData::load(const json& jsonObject, Progressable& progressable)
{
    uint64_t i = 0;

    if(!jsonObject["vectors"].is_array())
        return false;

    const auto& vectorsObject = jsonObject["vectors"];
    for(const auto& vector : vectorsObject)
    {
        if(!vector.is_object())
            return false;

        auto it = vector.begin();

        const QString& name = QString::fromStdString(it.key());
        const auto& jsonVector = it.value();

        UserDataVector userDataVector;
        if(!userDataVector.load(name, jsonVector))
            return false;

        _vectorNames.emplace_back(name);
        _userDataVectors.emplace(name, userDataVector);

        progressable.setProgress(static_cast<int>((i++ * 100) / vectorsObject.size()));
    }

    progressable.setProgress(-1);

    for(const auto& userDataVector : _userDataVectors)
        _numValues = std::max(_numValues, userDataVector.second.numValues());

    return true;
}
