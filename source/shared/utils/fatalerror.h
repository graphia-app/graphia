#ifndef FATALERROR_H
#define FATALERROR_H

// Deliberately crash in order to help track down bugs
// MESSAGE becomes a struct name, so quotes and spaces are not allowed
#define FATAL_ERROR(MESSAGE) \
    do { struct MESSAGE \
        { \
            void operator()() \
            { \
                int* p = nullptr; \
                *p = 0; \
            } \
        } s; s(); \
    } while(0)

#endif // FATALERROR_H
