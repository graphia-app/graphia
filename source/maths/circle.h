#ifndef CIRCLE
#define CIRCLE

#include <QRectF>

class Circle
{
public:
    Circle() {}

    Circle(float x, float y, float radius) noexcept :
        _x(x),
        _y(y),
        _radius(radius)
    {}

    Circle(const Circle& other) noexcept :
        _x(other._x),
        _y(other._y),
        _radius(other._radius)
    {}

    Circle(Circle&& other) noexcept :
        _x(other._x),
        _y(other._y),
        _radius(other._radius)
    {}

    Circle& operator=(const Circle& other) noexcept
    {
        _x = other._x;
        _y = other._y;
        _radius = other._radius;

        return *this;
    }

    Circle& operator=(Circle&& other) noexcept
    {
        _x = other._x;
        _y = other._y;
        _radius = other._radius;

        return *this;
    }

    float x() const { return _x; }
    float y() const { return _y; }
    float radius() const { return _radius; }

    QPointF centre() const { return QPointF(_x, _y); }

    QRectF boundingBox() const
    {
        return QRectF(_x - _radius, _y - _radius, _radius * 2.0f, _radius * 2.0f);
    }

    void set(float x, float y, float radius)
    {
        _x = x;
        _y = y;
        _radius = radius;
    }

    void setX(float x) { _x = x; }
    void setY(float y) { _y = y; }
    void setRadius(float radius) { _radius = radius; }

    void translate(const QPointF& translation)
    {
        _x += translation.x();
        _y += translation.y();
    }

    // Scales around origin
    void scale(float f)
    {
        _x *= f;
        _y *= f;
        _radius *= f;
    }

private:
    float _x = 0.0f;
    float _y = 0.0f;
    float _radius = 1.0f;
};

#endif // CIRCLE

