#ifndef SIZEVISUALISATIONCHANNEL_H
#define SIZEVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class SizeVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const;
    void apply(const QString&, ElementVisual&) const {} //FIXME

    bool supports(ValueType type) const { return type == ValueType::Int || type == ValueType::Float; }

    QString description(ElementType, ValueType) const;
};

#endif // SIZEVISUALISATIONCHANNEL_H
