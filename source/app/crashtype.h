#ifndef CRASHTYPE_H
#define CRASHTYPE_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(
      Q_GADGET, CrashType,
                NullPtrDereference,
                CppException,
                Win32Exception,
                Win32ExceptionNonContinuable);

#endif // CRASHTYPE_H
