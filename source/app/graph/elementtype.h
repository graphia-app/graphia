#ifndef ELEMENTTYPE_H
#define ELEMENTTYPE_H

#include "utils/qmlenum.h"

DEFINE_QML_ENUM(Q_GADGET, ElementType,
                None,
                Node = 0x1,
                Edge = 0x2,
                Component = 0x4,
                All = Node|Edge|Component);

QString elementTypeAsString(ElementType elementType);

#endif // ELEMENTTYPE_H
