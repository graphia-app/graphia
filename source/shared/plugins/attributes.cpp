#include "attributes.h"

Attribute& Attributes::attributeByName(const QString& name)
{
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&name](auto& v) { return v.name() == name; });
    Q_ASSERT(it != _attributes.end());
    return *it;
}

const Attribute& Attributes::attributeByName(const QString& name) const
{
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&name](auto& v) { return v.name() == name; });
    Q_ASSERT(it != _attributes.end());
    return *it;
}

int Attributes::size() const
{
    int maxSize = 0;

    for(auto& attribute : _attributes)
        maxSize = std::max(maxSize, attribute.size());

    return maxSize;
}

void Attributes::reserve(int size)
{
    for(auto& attribute : _attributes)
        attribute.reserve(size);
}

void Attributes::add(const QString& name)
{
    // Prevent adding the same attribute more than once
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&name](auto& v) { return v.name() == name; });
    if(it != _attributes.end())
        return;

    _attributes.emplace_back(name);
    emit attributeAdded(name);
}

void Attributes::setValue(int index, const QString& name, const QString& value)
{
    attributeByName(name).set(index, value);
}

QString Attributes::value(int index, const QString& name) const
{
    return attributeByName(name).get(index);
}
