#ifndef VISUALISATIONCONFIG_H
#define VISUALISATIONCONFIG_H

#include <QVariantMap>

struct VisualisationConfig
{
    QString _dataFieldName;
    QString _channelName;

    QVariantMap asVariantMap() const;

    bool operator==(const VisualisationConfig& other) const;
    bool operator!=(const VisualisationConfig& other) const;
};

#endif // VISUALISATIONCONFIG_H
