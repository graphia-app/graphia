#ifndef UTILS_H
#define UTILS_H

#include "pair_iterator.h"

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QColor>
#include <QString>

#include <type_traits>
#include <algorithm>
#include <vector>
#include <mutex>
#include <string>

namespace u
{
    float rand(float low, float high);
    int rand(int low, int high);

    QVector2D randQVector2D(float low, float high);
    QVector3D randQVector3D(float low, float high);
    QColor randQColor();

    template<typename T> T interpolate(const T& a, const T& b, float f)
    {
        return a + ((b - a) * f);
    }

    template<typename T> bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }

    float fast_rsqrt(float number);
    QVector3D fastNormalize(const QVector3D& v);

    template<typename T> T clamp(T min, T max, T value)
    {
        if(value < min)
            return min;
        else if(value > max)
            return max;

        return value;
    }

    template<typename T> bool signsMatch(T a, T b)
    {
        return (a > 0 && b > 0) || (a <= 0 && b <= 0);
    }

    int smallestPowerOf2GreaterThan(int x);

    int currentThreadId();
    void setCurrentThreadName(const QString& name);
    const QString currentThreadName();

    QQuaternion matrixToQuaternion(const QMatrix4x4& m);

    template<typename T> void checkEqual(T a, T b)
    {
        // Hopefully this function call elides to nothing when in release mode
        Q_ASSERT(a == b);
        Q_UNUSED(a);
        Q_UNUSED(b);
    }

    template<typename C, typename T> void removeByValue(C& container, const T& value)
    {
        container.erase(std::remove(container.begin(), container.end(), value), container.end());
    }

    template<typename C, typename T> size_t indexOf(C& container, const T& value)
    {
        return std::distance(container.begin(), std::find(container.begin(), container.end(), value));
    }

    template<typename C, typename T>
    auto contains(const C& container, const T& value, int)
    -> decltype(container.find(value), bool())
    {
        return container.find(value) != container.end();
    }

    template<typename C, typename T>
    auto contains(const C& container, const T& value, long)
    -> decltype(std::find(container.begin(), container.end(), value), bool())
    {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

    template<typename C, typename T> bool contains(const C& container, const T& value)
    {
        return contains(container, value, 0);
    }

    template<typename C, typename T> bool containsKey(const C& container, const T& key)
    {
        return contains(key_wrapper<C>(container), key, 0);
    }

    template<typename C, typename T> bool containsValue(const C& container, const T& value)
    {
        return contains(value_wrapper<C>(container), value, 0);
    }

    template<typename T,
             template<typename, typename...> class C1, typename... Args1,
             template<typename, typename...> class C2, typename... Args2>
    std::vector<T> setDifference(const C1<T, Args1...>& a, const C2<T, Args2...>& b)
    {
        std::vector<T> result;

        for(const auto& value : a)
        {
            if(!contains(b, value))
                result.emplace_back(value);
        }

        return result;
    }

    template<typename T,
             template<typename, typename...> class C1, typename... Args1,
             template<typename, typename...> class C2, typename... Args2>
    bool setsDiffer(const C1<T, Args1...>& a, const C2<T, Args2...>& b)
    {
        if(a.size() != b.size())
            return true;

        for(const auto& value : a)
        {
            if(!contains(b, value))
                return true;
        }

        return false;
    }

    template<typename T,
             template<typename, typename...> class C1, typename... Args1,
             template<typename, typename...> class C2, typename... Args2>
    bool setsEqual(const C1<T, Args1...>& a, const C2<T, Args2...>& b)
    {
        return !setsDiffer(a, b);
    }

    template<typename T,
             template<typename, typename...> class C1, typename... Args1,
             template<typename, typename...> class C2, typename... Args2>
    std::vector<T> setIntersection(const C1<T, Args1...>& a, const C2<T, Args2...>& b)
    {
        std::vector<T> result;

        for(const auto& value : a)
        {
            if(contains(b, value))
                result.emplace_back(value);
        }

        return result;
    }

    struct Locking {};

    // A lock that does nothing if the second parameter isn't Locking
    template<typename Mutex, typename L> struct MaybeLock
    {
        explicit MaybeLock(Mutex&) {}
    };

    template<typename Mutex> struct MaybeLock<Mutex, Locking>
    {
        std::unique_lock<Mutex> _lock;
        explicit MaybeLock(Mutex& mutex) : _lock(mutex) {}
    };

    template<typename C> int count(const C& c)
    {
        int n = 0;

        for(auto& i : c)
        {
            Q_UNUSED(i);
            n++;
        }

        return n;
    }

    bool isNumeric(const std::string& string);
}

#define ARRAY_SIZEOF(x) (sizeof(x)/sizeof((x)[0]))

#endif // UTILS_H
