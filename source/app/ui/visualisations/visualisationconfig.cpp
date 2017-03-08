#include "visualisationconfig.h"

#include "shared/utils/utils.h"

QVariantMap VisualisationConfig::asVariantMap() const
{
    QVariantMap map;

    QVariantList flags;
    for(const auto& flag : _flags)
        flags.append(flag);
    map.insert("flags", flags);

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

bool VisualisationConfig::isFlagSet(const QString& flag) const
{
    return u::contains(_flags, flag);
}
