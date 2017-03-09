#ifndef FIELDTYPE_H
#define FIELDTYPE_H

#include "utils/qmlenum.h"

DEFINE_QML_ENUM(Q_GADGET, FieldType,
                Unknown,
                Int = 0x1,
                Float = 0x2,
                String = 0x4,
                All = Int|Float|String);

#endif // FIELDTYPE_H
