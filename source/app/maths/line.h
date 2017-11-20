#ifndef LINE_H
#define LINE_H

#include <QVector2D>
#include <QVector3D>

#include <utility>

template <typename Vector> class Line
{
private:
    Vector _start;
    Vector _end;
    mutable float _length = -1.0f;

public:
    Line() = default;
    Line(Vector start, Vector end) :
        _start(std::move(start)), _end(std::move(end))
    {}

    const Vector& start() const { return _start; }
    const Vector& end() const { return _end; }

    Vector dir() const { return (_end - _start).normalized(); }

    float length() const
    {
        // Calculate on demand
        if(_length < 0.0f)
            _length = _start.distanceToPoint(_end);

        return _length;
    }

    void setStart(const Vector& start)
    {
        _start = start;
        _length = -1.0f;
    }

    void setEnd(const Vector& end)
    {
        _end = end;
        _length = -1.0f;
    }
};

using Line3D = Line<QVector3D>;
using Line2D = Line<QVector2D>;

#endif // LINE_H
