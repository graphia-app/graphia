#ifndef MAKE_UNIQUE_H
#define MAKE_UNIQUE_H

#ifndef _MSC_VER
#if __cplusplus <= 201103L
namespace std {
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}
}
#else
#error std::make_unique should now be available
#endif
#endif

#endif // MAKE_UNIQUE_H
