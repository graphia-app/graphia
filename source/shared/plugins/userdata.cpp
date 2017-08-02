#include "userdata.h"

#include "shared/utils/utils.h"

#include <QJsonArray>

const QString UserData::firstUserDataVectorName() const
{
    if(!_userDataVectors.empty())
        return _userDataVectors.at(0).first;

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

void UserData::add(const QString& name)
{
    if(!u::containsKey(_userDataVectors, name))
    {
        _userDataVectors.emplace_back(std::make_pair(name, UserDataVector(name)));
        emit userDataVectorAdded(name);
    }
}

void UserData::setValue(size_t index, const QString& name, const QString& value)
{
    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
                           [&name](const auto& it2) { return it2.first == name; });

    if(it != _userDataVectors.end())
    {
        it->second.set(index, value);
        _numValues = std::max(_numValues, it->second.numValues());
    }
}

QVariant UserData::value(size_t index, const QString& name) const
{
    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
                           [&name](const auto& it2) { return it2.first == name; });

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

QJsonObject UserData::save(const ProgressFn& progressFn) const
{
    QJsonObject jsonObject;
    int i = 0;

    jsonObject["numValues"] = _numValues;

    QJsonArray vectors;
    for(const auto& userDataVector : _userDataVectors)
    {
        const auto& name = userDataVector.first;
        auto jsonVector = userDataVector.second.save();

        QJsonObject vector;
        vector["name"] = name;
        vector["vector"] = jsonVector;

        vectors.append(vector);
        progressFn((i++ * 100) / static_cast<int>(_userDataVectors.size()));
    }

    jsonObject["vectors"] = vectors;

    progressFn(-1);

    return jsonObject;
}

bool UserData::load(const QJsonObject& jsonObject, const ProgressFn& progressFn)
{
    int i = 0;

    if(!jsonObject["vectors"].isArray())
        return false;

    _userDataVectors.clear();
    _numValues = 0;

    const auto& vectorsArray = jsonObject["vectors"].toArray();
    for(const auto& vector : vectorsArray)
    {
        const auto& name = vector.toObject()["name"].toString();
        const auto& jsonVector = vector.toObject()["vector"].toObject();

        UserDataVector userDataVector;
        if(!userDataVector.load(jsonVector))
            return false;

        _userDataVectors.emplace_back(std::make_pair(name, userDataVector));

        progressFn((i++ * 100) / vectorsArray.size());
    }

    progressFn(-1);

    for(const auto& userDataVector : _userDataVectors)
        _numValues = std::max(_numValues, userDataVector.second.numValues());

    return true;
}
