#ifndef ENUMREFLECTION_H
#define ENUMREFLECTION_H

#include <QString>

template<typename T> struct EnumStrings
{
    static QString values[];
    static size_t size;
};

#define DECLARE_REFLECTED_ENUM(E) /* NOLINT cppcoreguidelines-macro-usage */ \
    template<> struct EnumStrings<E> \
    { \
        static QString values[]; \
        static size_t size; \
    };

#define DEFINE_REFLECTED_ENUM(E, ...)  /* NOLINT cppcoreguidelines-macro-usage */ \
    QString EnumStrings<E>::values[] = \
    {__VA_ARGS__}; \
    size_t EnumStrings<E>::size = \
        sizeof(EnumStrings<E>::values) / sizeof(EnumStrings<E>::values[0]);

template<typename T>
const QString& enumToString(T value)
{
    auto index = static_cast<size_t>(value);
    Q_ASSERT(index < EnumStrings<T>::size);
    return EnumStrings<T>::values[index];
}

template<typename T>
T stringToEnum(const QString& value)
{
    for(size_t i = 0; i < EnumStrings<T>::size; i++)
    {
        if(EnumStrings<T>::values[i] == value)
            return static_cast<T>(i);
    }

    return T();
}

#endif // ENUMREFLECTION_H

