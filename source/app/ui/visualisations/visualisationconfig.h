#ifndef VISUALISATIONCONFIG_H
#define VISUALISATIONCONFIG_H

#include <QString>
#include <QVariantMap>

#include <vector>

struct VisualisationConfig
{
    std::vector<QString> _metaAttributes;
    QString _dataFieldName;
    QString _channelName;

    QVariantMap asVariantMap() const;

    bool operator==(const VisualisationConfig& other) const;
    bool operator!=(const VisualisationConfig& other) const;
    bool isMetaAttributeSet(const QString& metaAttribute) const;
};

#endif // VISUALISATIONCONFIG_H
