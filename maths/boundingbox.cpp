#include "boundingbox.h"

#include <algorithm>

BoundingBox2D::BoundingBox2D()
    : _min(), _max()
{}

BoundingBox2D::BoundingBox2D(const QVector2D& min, const QVector2D& max)
    : _min(min), _max(max)
{}

float BoundingBox2D::maxLength() const
{
    return std::max(xLength(), yLength());
}

void BoundingBox2D::set(const QVector2D& min, const QVector2D& max)
{
    _min = min;
    _max = max;
}

static QVector2D minQVector2D(const QVector2D& a, const QVector2D& b)
{
    QVector2D v;

    v.setX(std::min(a.x(), b.x()));
    v.setY(std::min(a.y(), b.y()));

    return v;
}

static QVector2D maxQVector2D(const QVector2D& a, const QVector2D& b)
{
    QVector2D v;

    v.setX(std::max(a.x(), b.x()));
    v.setY(std::max(a.y(), b.y()));

    return v;
}

void BoundingBox2D::expandToInclude(const QVector2D& point)
{
    _min = minQVector2D(_min, point);
    _max = maxQVector2D(_max, point);
}

void BoundingBox2D::expandToInclude(const BoundingBox2D& other)
{
    _min = minQVector2D(_min, other._min);
    _max = maxQVector2D(_max, other._max);
}

bool BoundingBox2D::containsPoint(const QVector2D& point) const
{
    return (point.x() >= _min.x() && point.x() < _max.x()) &&
            (point.y() >= _min.y() && point.y() < _max.y());
}

bool BoundingBox2D::containsLine(const QVector2D& pointA, const QVector2D& pointB) const
{
    return containsPoint(pointA) && containsPoint(pointB);
}

QVector2D BoundingBox2D::centre() const
{
    return (_min + _max) / 2.0f;
}

BoundingBox3D::BoundingBox3D()
    : _min(), _max()
{}

BoundingBox3D::BoundingBox3D(const QVector3D& min, const QVector3D& max)
    : _min(min), _max(max)
{}

float BoundingBox3D::maxLength() const
{
    return std::max(std::max(xLength(), yLength()), zLength());
}

void BoundingBox3D::set(const QVector3D& min, const QVector3D& max)
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

void BoundingBox3D::expandToInclude(const QVector3D& point)
{
    _min = minQVector3D(_min, point);
    _max = maxQVector3D(_max, point);
}

void BoundingBox3D::expandToInclude(const BoundingBox3D& other)
{
    _min = minQVector3D(_min, other._min);
    _max = maxQVector3D(_max, other._max);
}

bool BoundingBox3D::containsPoint(const QVector3D& point) const
{
    return (point.x() >= _min.x() && point.x() < _max.x()) &&
            (point.y() >= _min.y() && point.y() < _max.y()) &&
            (point.z() >= _min.z() && point.z() < _max.z());
}

bool BoundingBox3D::containsLine(const QVector3D& pointA, const QVector3D& pointB) const
{
    return containsPoint(pointA) && containsPoint(pointB);
}

QVector3D BoundingBox3D::centre() const
{
    return (_min + _max) / 2.0f;
}
