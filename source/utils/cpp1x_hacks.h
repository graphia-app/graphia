#ifndef CPP1X_HACKS_H
#define CPP1X_HACKS_H

#ifndef _MSC_VER
#if __cplusplus <= 201103L
#include <memory>

namespace std
{
    template<typename T, typename ...Args>
    std::unique_ptr<T> make_unique(Args&& ...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#else
#error std::make_unique should now be available
#endif
#endif

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER <= 180031101
#include <yvals.h>
#ifdef _NOEXCEPT
#define noexcept _NOEXCEPT
#endif

// No constexpr yet, apparently
#define constexpr
#endif

#endif // CPP1X_HACKS_H
