#ifndef SIZEVISUALISATIONCHANNEL_H
#define SIZEVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class SizeVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString&, ElementVisual&) const override {} //FIXME

    bool supports(ValueType type) const override { return type == ValueType::Int || type == ValueType::Float; }

    QString description(ElementType, ValueType) const override;
};

#endif // SIZEVISUALISATIONCHANNEL_H
