#ifndef VALUETYPE_H
#define VALUETYPE_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(Q_GADGET, ValueType,
                Unknown     = 0x1,
                Int         = 0x2,
                Float       = 0x4,
                String      = 0x8,
                All = Int|Float|String);

#endif // VALUETYPE_H
