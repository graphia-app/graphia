#include "attributes.h"

#include "shared/utils/utils.h"

const QString Attributes::firstAttributeName() const
{
    if(!_attributes.empty())
        return _attributes.at(0).first;

    return {};
}

int Attributes::numAttributes() const
{
    return static_cast<int>(_attributes.size());
}

int Attributes::numValues() const
{
    int maxSize = 0;

    for(auto& attribute : _attributes)
        maxSize = std::max(maxSize, attribute.second.numValues());

    return maxSize;
}

void Attributes::add(const QString& name)
{
    if(_attributes.empty())
        _firstAttributeName = name;

    if(!u::containsKey(_attributes, name))
    {
        _attributes.emplace_back(std::make_pair(name, Attribute(name)));
        emit attributeAdded(name);
    }
}

void Attributes::setValue(int index, const QString& name, const QString& value)
{
    auto attributeIt = std::find_if(_attributes.begin(), _attributes.end(),
                                    [&name](const auto& it) { return it.first == name; });

    if(attributeIt != _attributes.end())
        attributeIt->second.set(index, value);
}

QString Attributes::value(int index, const QString& name) const
{
    auto attributeIt = std::find_if(_attributes.begin(), _attributes.end(),
                                    [&name](const auto& it) { return it.first == name; });

    if(attributeIt != _attributes.end())
        return attributeIt->second.get(index);

    return {};
}
