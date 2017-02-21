#include "visualisationconfig.h"

QVariantMap VisualisationConfig::asVariantMap() const
{
    QVariantMap map;

    map.insert("dataField", _dataFieldName);
    map.insert("channel", _channelName);

    return map;
}

bool VisualisationConfig::operator==(const VisualisationConfig& other) const
{
    return _dataFieldName == other._dataFieldName &&
            _channelName == other._channelName;
}

bool VisualisationConfig::operator!=(const VisualisationConfig& other) const
{
    return !operator==(other);
}
