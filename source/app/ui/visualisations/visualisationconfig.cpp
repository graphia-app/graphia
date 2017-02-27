#include "visualisationconfig.h"

#include "shared/utils/utils.h"

QVariantMap VisualisationConfig::asVariantMap() const
{
    QVariantMap map;

    QVariantList metaAttributes;
    for(const auto& metaAttribute : _metaAttributes)
        metaAttributes.append(metaAttribute);
    map.insert("metaAttributes", metaAttributes);

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

bool VisualisationConfig::isMetaAttributeSet(const QString& metaAttribute) const
{
    return u::contains(_metaAttributes, metaAttribute);
}
