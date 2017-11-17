#ifndef COLORVISUALISATIONCHANNEL_H
#define COLORVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"
#include "colorgradient.h"

class ColorVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString& value, ElementVisual& elementVisual) const override;

    bool supports(ValueType valueType) const override { return valueType != ValueType::Unknown; }

    QString description(ElementType, ValueType) const override;

    QVariantMap defaultParameters(ValueType valueType) const override;
    void setParameter(const QString& name, const QString& value) override;

private:
    ColorGradient _colorGradient;
};

#endif // COLORVISUALISATIONCHANNEL_H
