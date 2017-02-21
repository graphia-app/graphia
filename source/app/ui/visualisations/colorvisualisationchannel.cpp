#include "colorvisualisationchannel.h"

void ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    QColor from(Qt::red);
    QColor to(Qt::yellow);
    QColor diff;

    auto redDiff = to.redF() -     from.redF();
    auto greenDiff = to.greenF() - from.greenF();
    auto blueDiff = to.blueF() -   from.blueF();

    elementVisual._color.setRedF(from.redF() +     (value * redDiff));
    elementVisual._color.setGreenF(from.greenF() + (value * greenDiff));
    elementVisual._color.setBlueF(from.blueF() +   (value * blueDiff));
}

void ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    auto hash = qHash(value);

    elementVisual._color.setHsv(static_cast<int>(hash % 255), 255, 255);
}
