#include "userdata.h"

#include "shared/utils/utils.h"

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
    int maxSize = 0;

    for(auto& userDataVector : _userDataVectors)
        maxSize = std::max(maxSize, userDataVector.second.numValues());

    return maxSize;
}

void UserData::add(const QString& name)
{
    if(_userDataVectors.empty())
        _firstUserDataVectorName = name;

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
        it->second.set(index, value);
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
