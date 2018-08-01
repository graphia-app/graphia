#include "thirdparty/gccdiagaware.h"

#ifdef GCC_DIAGNOSTIC_AWARE
// GCC/clang warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#ifdef __clang__
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#endif
#endif

#ifdef _MSC_VER
// MSVC warnings
#pragma warning( push )
#pragma warning( disable : 6387 ) // '...' could be '0': this does not adhere to the specification for the function 'memcpy'
#endif
