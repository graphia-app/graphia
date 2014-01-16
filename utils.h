#ifndef UTILS_H
#define UTILS_H

#include <QVector3D>
#include <QColor>
#include <cstdlib>
#include <cmath>

class Utils
{
public:
    template<typename T> static T rand(T low, T high)
    {
        T range = high - low;
        int64_t randValue = ::rand();
        int64_t numerator = randValue * range;
        T value = ( numerator / static_cast<T>(RAND_MAX)) + low;

        return value;
    }

    static QVector2D randQVector2D(float low, float high)
    {
        return QVector2D(rand(low, high), rand(low, high));
    }

    static QVector3D randQVector3D(float low, float high)
    {
        return QVector3D(rand(low, high), rand(low, high), rand(low, high));
    }

    static QColor randQColor()
    {
        int r = rand(0, 255);
        int g = rand(0, 255);
        int b = rand(0, 255);

        return QColor(r, g, b);
    }

    template<typename T> static bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }

    static float fast_rsqrt(float number)
    {
        typedef union
        {
            float f;
            int i;
            unsigned int ui;
        } floatint_t;

        floatint_t t;
        float x2, y;
        const float threehalfs = 1.5F;

        x2 = number * 0.5F;
        t.f  = number;
        t.i  = 0x5f3759df - ( t.i >> 1 );               // what the fuck?
        y  = t.f;
        y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
    //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

        return y;
    }

    static QVector3D FastNormalize(QVector3D& v)
    {
        float xp = v.x(); float yp = v.y(); float zp = v.z();
        // Need some extra precision if the length is very small.
        double len = double(xp) * double(xp) +
                     double(yp) * double(yp) +
                     double(zp) * double(zp);
        if (qFuzzyIsNull(len - 1.0f)) {
            return v;
        } else if (!qFuzzyIsNull(len)) {
            float rsqrtLen = fast_rsqrt(len);
            return QVector3D(xp * rsqrtLen,
                             yp * rsqrtLen,
                             zp * rsqrtLen);
        } else {
            return QVector3D();
        }
    }

    static constexpr float Pi() { return std::atan2(0, -1); }
};

#endif // UTILS_H
