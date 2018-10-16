#ifndef FATALERROR_H
#define FATALERROR_H

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NOINLINE __attribute__ ((noinline))
#endif

// Deliberately crash in order to help track down bugs
// MESSAGE becomes a struct name, so quotes and spaces are not allowed
#define FATAL_ERROR(MESSAGE) \
    do { struct MESSAGE \
        { \
            NOINLINE void operator()() \
            { \
                /* cppcheck-suppress nullPointer */ \
                int* p = nullptr; \
                *p = 0; \
            } \
        } s; s(); \
    } while(0)

#endif // FATALERROR_H
