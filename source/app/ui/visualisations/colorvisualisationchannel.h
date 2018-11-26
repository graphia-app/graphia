#ifndef COLORVISUALISATIONCHANNEL_H
#define COLORVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"
#include "colorgradient.h"
#include "colorpalette.h"

#include <vector>
#include <QString>

class ColorVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString& value, ElementVisual& elementVisual) const override;

    bool supports(ValueType valueType) const override { return valueType != ValueType::Unknown; }

    QString description(ElementType, ValueType) const override;

    void reset() override;
    QVariantMap defaultParameters(ValueType valueType) const override;
    void setParameter(const QString& name, const QString& value) override;

private:
    ColorGradient _colorGradient;
    ColorPalette _colorPalette;
    std::vector<QString> _sharedValues;
};

#endif // COLORVISUALISATIONCHANNEL_H
