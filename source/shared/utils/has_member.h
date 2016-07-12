#ifndef HASMEMBER_H
#define HASMEMBER_H

// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector

#include <type_traits>
#include <iostream>
#include <iomanip>

#define GENERATE_HAS_MEMBER(member)                                               \
template<typename T>                                                              \
class HasMember_##member                                                          \
{                                                                                 \
private:                                                                          \
    using Yes = char[2];                                                          \
    using No = char[1];                                                           \
                                                                                  \
    struct Fallback { int member; };                                              \
    struct Derived : T, Fallback {};                                              \
                                                                                  \
    template<class U>                                                             \
    static No& test(decltype(U::member)*);                                        \
    template<typename U>                                                          \
    static Yes& test(U*);                                                         \
                                                                                  \
public:                                                                           \
    static constexpr bool RESULT = sizeof(test<Derived>(nullptr)) == sizeof(Yes); \
};                                                                                \
                                                                                  \
template<typename T>                                                              \
struct has_member_##member                                                        \
: public std::integral_constant<bool, HasMember_##member<T>::RESULT>              \
{                                                                                 \
};

#endif // HASMEMBER_H
