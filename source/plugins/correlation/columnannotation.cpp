#include "columnannotation.h"

ColumnAnnotation::ColumnAnnotation(QString name, std::vector<QString> values) :
    _name(std::move(name)), _values(std::move(values))
{
    for(const auto& value : _values)
        _uniqueValues.emplace(value);
}
