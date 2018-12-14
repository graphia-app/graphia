#include "visualisationchannel.h"

#include "shared/utils/container.h"

void VisualisationChannel::reset()
{
    _values.clear();
    _valueIndexMap.clear();
}

void VisualisationChannel::addValue(const QString& value)
{
    _values.emplace_back(value);
    _valueIndexMap[value] = static_cast<int>(_values.size()) - 1;
}

int VisualisationChannel::indexOf(const QString& value) const
{
    if(u::contains(_valueIndexMap, value))
        return static_cast<int>(_valueIndexMap.at(value));

    return -1;
}
