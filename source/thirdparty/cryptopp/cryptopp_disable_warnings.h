// cryptopp is a very warning noisy library, so we allow some warnings to be disabled
// by including this header before including any crypto headers. The companion to
// this is crypto_enable_warnings.h which should be included as soon as possible
// after the affected code; especially if we're including from inside another header.

#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) || \
    defined(__clang__))
#define GCC_DIAGNOSTIC_AWARE
#endif

#ifdef GCC_DIAGNOSTIC_AWARE
// GCC/clang warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wshadow"

#if defined(__clang__) && defined(__APPLE__)
// clang specific warnings
#pragma GCC diagnostic ignored "-Wundefined-var-template"
#endif

#endif

#ifdef _MSC_VER
// MSVC warnings
#pragma warning( push )
#endif
