#ifndef VALUETYPE_H
#define VALUETYPE_H

#include "utils/qmlenum.h"

DEFINE_QML_ENUM(Q_GADGET, ValueType,
                Unknown,
                Int = 0x1,
                Float = 0x2,
                String = 0x4,
                All = Int|Float|String);

#endif // VALUETYPE_H
