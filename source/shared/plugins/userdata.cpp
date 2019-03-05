#include "userdata.h"

#include "shared/utils/container.h"

QString UserData::firstUserDataVectorName() const
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

std::vector<QString> UserData::vectorNames() const
{
    return _vectorNames;
}

UserDataVector& UserData::add(QString name)
{
    if(name.isEmpty())
        name = QObject::tr("Unnamed");

    if(!u::contains(_vectorNames, name))
    {
        _vectorNames.emplace_back(name);
        _userDataVectors.emplace_back(std::make_pair(name, UserDataVector(name)));
        return _userDataVectors.back().second;
    }

    auto it = std::find_if(_userDataVectors.begin(), _userDataVectors.end(),
                           [&name](const auto& it2) { return it2.first == name; });

    Q_ASSERT(it != _userDataVectors.end());

    return it->second;
}

void UserData::setValue(size_t index, const QString& name, const QString& value)
{
    auto& userDataVector = add(name);

    userDataVector.set(index, value);
    _numValues = std::max(_numValues, userDataVector.numValues());
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

json UserData::save(Progressable& progressable, const std::vector<size_t>& indexes) const
{
    json jsonObject;

    int i = 0;

    std::vector<std::map<std::string, json>> vectors;
    for(const auto& userDataVector : _userDataVectors)
    {
        std::map<std::string, json> vector;
        const auto& name = userDataVector.first.toStdString();
        vector.emplace(name, userDataVector.second.save(indexes));
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

    _userDataVectors.clear();
    _numValues = 0;

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
        _userDataVectors.emplace_back(std::make_pair(name, userDataVector));

        progressable.setProgress(static_cast<int>((i++ * 100) / vectorsObject.size()));
    }

    progressable.setProgress(-1);

    for(const auto& userDataVector : _userDataVectors)
        _numValues = std::max(_numValues, userDataVector.second.numValues());

    return true;
}
