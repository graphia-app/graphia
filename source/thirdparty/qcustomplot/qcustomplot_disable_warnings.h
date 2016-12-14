#define GCC_DIAGNOSTIC_AWARE \
    ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) || \
    defined(__clang__))

#if GCC_DIAGNOSTIC_AWARE
// GCC/clang warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif

#ifdef _MSC_VER
// MSVC warnings
#endif
