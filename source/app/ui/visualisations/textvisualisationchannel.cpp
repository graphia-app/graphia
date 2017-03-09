#include "textvisualisationchannel.h"

#include <QObject>

void TextVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._text = QString::number(value, 'g', 3);
}

void TextVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    elementVisual._text = value;
}

QString TextVisualisationChannel::description(ElementType, ValueType) const
{
    return QObject::tr("The attribute will be visualised as text.");
}
