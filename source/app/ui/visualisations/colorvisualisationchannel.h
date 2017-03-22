#ifndef COLORVISUALISATIONCHANNEL_H
#define COLORVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"
#include "colorgradient.h"

class ColorVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const;
    void apply(const QString& value, ElementVisual& elementVisual) const;

    bool supports(ValueType valueType) const { return valueType != ValueType::Unknown; }

    QString description(ElementType, ValueType) const;

    void resetParameters();
    void setParameter(const QString& name, const QString& value);

private:
    ColorGradient _colorGradient;
};

#endif // COLORVISUALISATIONCHANNEL_H
