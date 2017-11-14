#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) || \
    defined(__clang__))
#define GCC_DIAGNOSTIC_AWARE
#endif

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
#endif
