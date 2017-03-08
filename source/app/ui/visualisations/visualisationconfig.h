#ifndef VISUALISATIONCONFIG_H
#define VISUALISATIONCONFIG_H

#include <QString>
#include <QVariantMap>

#include <vector>

struct VisualisationConfig
{
    std::vector<QString> _flags;
    QString _attributeName;
    QString _channelName;

    QVariantMap asVariantMap() const;

    bool operator==(const VisualisationConfig& other) const;
    bool operator!=(const VisualisationConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // VISUALISATIONCONFIG_H
