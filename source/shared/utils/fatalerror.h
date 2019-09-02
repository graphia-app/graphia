#ifndef FATALERROR_H
#define FATALERROR_H

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NOINLINE __attribute__ ((noinline)) /* NOLINT cppcoreguidelines-macro-usage */
#endif

// Deliberately crash in order to help track down bugs
// MESSAGE becomes a struct name, so quotes and spaces are not allowed
#define FATAL_ERROR(MESSAGE) /* NOLINT cppcoreguidelines-macro-usage */ \
    do { struct MESSAGE \
        { \
            NOINLINE void operator()() \
            { \
                int* p = nullptr; \
                *p = 0; \
            } \
        } s; s(); \
    } while(0)

#endif // FATALERROR_H
