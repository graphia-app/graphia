#include "visualisationchannel.h"

void VisualisationChannel::reset()
{
    _values.clear();
}

void VisualisationChannel::addValue(const QString& value)
{
    _values.emplace_back(value);
}
