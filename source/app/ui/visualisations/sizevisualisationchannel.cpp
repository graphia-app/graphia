#include "sizevisualisationchannel.h"

#include <QObject>

bool SizeVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    bool alreadySet = elementVisual._size >= 0.0f;

    elementVisual._size = value;

    return alreadySet;
}

QString SizeVisualisationChannel::description(ElementType elementType, FieldType) const
{
    auto elementTypeString = elementTypeAsString(elementType).toLower();
    return QString(QObject::tr("The attribute will be visualised by "
                      "varying the size of the %1.")).arg(elementTypeString);
}
