#ifndef GCC_DIAG_AWARE
#define GCC_DIAG_AWARE

#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) || \
    defined(__clang__))
#define GCC_DIAGNOSTIC_AWARE
#endif

#endif // GCC_DIAG_AWARE
