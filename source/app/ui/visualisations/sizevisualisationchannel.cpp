#include "sizevisualisationchannel.h"

void SizeVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._size = value;
}
