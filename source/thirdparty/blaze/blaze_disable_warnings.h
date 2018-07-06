#include "thirdparty/gccdiagaware.h"

#ifdef GCC_DIAGNOSTIC_AWARE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#endif

#ifdef _MSC_VER
#if _MSC_VER <= 1900
#pragma warning( push )
#pragma warning( disable : 4100 ) // Unreferenced formal parameter
#endif
#endif
