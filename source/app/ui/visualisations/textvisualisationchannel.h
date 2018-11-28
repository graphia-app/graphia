#ifndef TEXTVISUALISATIONCHANNEL_H
#define TEXTVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class TextVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString& value, ElementVisual& elementVisual) const override;

    bool supports(ValueType valueType) const override { return valueType != ValueType::Unknown; }
    bool requiresNormalisedValue() const override { return false; }
    bool requiresRange() const override { return false; }

    void findErrors(VisualisationInfo& info) const override;

    QString description(ElementType, ValueType) const override;
};

#endif // TEXTVISUALISATIONCHANNEL_H
