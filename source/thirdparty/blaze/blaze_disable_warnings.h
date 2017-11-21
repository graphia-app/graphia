#include "thirdparty/gccdiagaware.h"

#ifdef GCC_DIAGNOSTIC_AWARE

#if defined(__clang__)
// clang specific warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#endif
