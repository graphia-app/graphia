#ifndef SIZEVISUALISATIONCHANNEL_H
#define SIZEVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class SizeVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const;
    void apply(const QString&, ElementVisual&) const {} //FIXME

    bool supports(FieldType type) { return type == FieldType::Int || type == FieldType::Float; }
};

#endif // SIZEVISUALISATIONCHANNEL_H
