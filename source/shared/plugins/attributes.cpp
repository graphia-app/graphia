#include "attributes.h"

int Attributes::size() const
{
    int maxSize = 0;

    for(auto& attribute : _attributes)
        maxSize = std::max(maxSize, attribute.second.size());

    return maxSize;
}

void Attributes::add(const QString& name)
{
    if(_attributes.empty())
        _firstAttributeName = name;

    _attributes.emplace(name, Attribute(name));
    emit attributeAdded(name);
}

void Attributes::setValue(int index, const QString& name, const QString& value)
{
    _attributes[name].set(index, value);
}

QString Attributes::value(int index, const QString& name) const
{
    return _attributes.at(name).get(index);
}
