#include "boundingbox.h"
#include "ray.h"

#include <algorithm>
#include <limits>

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

void BoundingBox3D::scale(float s)
{
    QVector3D newMin = ((_min - centre()) * s) + centre();
    QVector3D newMax = ((_max - centre()) * s) + centre();

    _min = newMin;
    _max = newMax;
}

BoundingBox3D BoundingBox3D::scaled(float s) const
{
    BoundingBox3D newBoundingBox(min(), max());
    newBoundingBox.scale(s);
    return newBoundingBox;
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

// From "An Efficient and Robust Ray–Box Intersection Algorithm"
bool BoundingBox3D::intersects(const Ray &ray, float t0, float t1) const
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    const QVector3D bounds[2] = { this->min(), this->max() };

    tmin = (bounds[ray.sign()[0]].x() - ray.origin().x()) * ray.invDir().x();
    tmax = (bounds[1-ray.sign()[0]].x() - ray.origin().x()) * ray.invDir().x();
    tymin = (bounds[ray.sign()[1]].y() - ray.origin().y()) * ray.invDir().y();
    tymax = (bounds[1-ray.sign()[1]].y() - ray.origin().y()) * ray.invDir().y();

    if((tmin > tymax) || (tymin > tmax))
        return false;

    if(tymin > tmin)
        tmin = tymin;

    if(tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[ray.sign()[2]].z() - ray.origin().z()) * ray.invDir().z();
    tzmax = (bounds[1-ray.sign()[2]].z() - ray.origin().z()) * ray.invDir().z();

    if((tmin > tzmax) || (tzmin > tmax))
        return false;

    if(tzmin > tmin)
        tmin = tzmin;

    if(tzmax < tmax)
        tmax = tzmax;

    return ((tmin < t1) && (tmax > t0));
}

bool BoundingBox3D::intersects(const Ray &ray) const
{
    return intersects(ray, 0.0f, std::numeric_limits<float>::max());
}

QVector3D BoundingBox3D::centre() const
{
    return (_min + _max) / 2.0f;
}
