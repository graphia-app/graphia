#ifndef TEXTVISUALISATIONCHANNEL_H
#define TEXTVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class TextVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const;
    void apply(const QString& value, ElementVisual& elementVisual) const;

    bool supports(ValueType valueType) const { return valueType != ValueType::Unknown; }
    bool requiresNormalisedValue() const { return false; }

    QString description(ElementType, ValueType) const;
};

#endif // TEXTVISUALISATIONCHANNEL_H
