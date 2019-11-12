#ifndef PROJECTION_H
#define PROJECTION_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(
   Q_GADGET, Projection,
   Unset = -1, Perspective, Orthographic, TwoDee);

#endif // PROJECTION_H
