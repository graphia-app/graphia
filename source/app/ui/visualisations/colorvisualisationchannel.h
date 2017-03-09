#ifndef COLORVISUALISATIONCHANNEL_H
#define COLORVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class ColorVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const;
    void apply(const QString& value, ElementVisual& elementVisual) const;

    bool supports(ValueType valueType) const { return valueType != ValueType::Unknown; }

    QString description(ElementType, ValueType) const;
};

#endif // COLORVISUALISATIONCHANNEL_H
