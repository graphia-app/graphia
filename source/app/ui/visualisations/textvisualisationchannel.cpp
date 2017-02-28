#include "textvisualisationchannel.h"

#include <QObject>

bool TextVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    bool alreadySet = !elementVisual._text.isEmpty();

    elementVisual._text = QString::number(value, 'g', 3);

    return alreadySet;
}

bool TextVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    bool alreadySet = !elementVisual._text.isEmpty();

    elementVisual._text = value;

    return alreadySet;
}

QString TextVisualisationChannel::description(ElementType, FieldType) const
{
    return QObject::tr("The attribute will be visualised as text.");
}
