#include "boundingbox.h"

#include <algorithm>

BoundingBox::BoundingBox()
    : _min(), _max()
{}

BoundingBox::BoundingBox(const QVector3D& min, const QVector3D& max)
    : _min(min), _max(max)
{}

float BoundingBox::maxLength() const
{
    return std::max(std::max(xLength(), yLength()), zLength());
}

void BoundingBox::set(const QVector3D& min, const QVector3D& max)
{
    _min = min;
    _max = max;
}

static QVector3D minQVector3D(const QVector3D& a, const QVector3D& b)
{
    QVector3D v;

    v.setX(std::min(a.x(), b.x()));
    v.setY(std::min(a.y(), b.y()));
    v.setZ(std::min(a.z(), b.z()));

    return v;
}

static QVector3D maxQVector3D(const QVector3D& a, const QVector3D& b)
{
    QVector3D v;

    v.setX(std::max(a.x(), b.x()));
    v.setY(std::max(a.y(), b.y()));
    v.setZ(std::max(a.z(), b.z()));

    return v;
}

void BoundingBox::expandToInclude(const QVector3D& point)
{
    _min = minQVector3D(_min, point);
    _max = maxQVector3D(_max, point);
}

void BoundingBox::expandToInclude(const BoundingBox& other)
{
    _min = minQVector3D(_min, other._min);
    _max = maxQVector3D(_max, other._max);
}

bool BoundingBox::containsPoint(const QVector3D& point) const
{
    return (point.x() >= _min.x() && point.x() < _max.x()) &&
            (point.y() >= _min.y() && point.y() < _max.y()) &&
            (point.z() >= _min.z() && point.z() < _max.z());
}

bool BoundingBox::containsLine(const QVector3D& pointA, const QVector3D& pointB) const
{
    return containsPoint(pointA) && containsPoint(pointB);
}

QVector3D BoundingBox::centre() const
{
    return (_min + _max) / 2.0f;
}
