#include "sizevisualisationchannel.h"

#include <QObject>

void SizeVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._size = value;
}

QString SizeVisualisationChannel::description(ElementType elementType, ValueType) const
{
    auto elementTypeString = elementTypeAsString(elementType).toLower();
    return QString(QObject::tr("The attribute will be visualised by "
                      "varying the size of the %1.")).arg(elementTypeString);
}
