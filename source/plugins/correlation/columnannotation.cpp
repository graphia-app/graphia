#include "columnannotation.h"

ColumnAnnotation::ColumnAnnotation(QString name, std::vector<QString> values) :
    _name(std::move(name)), _values(std::move(values))
{
    for(const auto& value : _values)
        _uniqueValues.emplace(value);
}

ColumnAnnotation::ColumnAnnotation(QString name,
    const ColumnAnnotation::Iterator& begin,
    const ColumnAnnotation::Iterator& end) :
    _name(std::move(name))
{
    for(auto it = begin; it != end; ++it)
    {
        _values.emplace_back(*it);
        _uniqueValues.emplace(*it);
    }
}
