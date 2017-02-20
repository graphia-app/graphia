#include "textvisualisationchannel.h"

void TextVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._text = QString::number(value);
}

void TextVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    elementVisual._text = value;
}
