#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "line.h"

#include <QVector2D>
#include <QVector3D>

#include <vector>

class BoundingBox2D
{
private:
    QVector2D _min;
    QVector2D _max;

public:
    BoundingBox2D();
    BoundingBox2D(QVector2D min, QVector2D max);
    explicit BoundingBox2D(const std::vector<QVector2D>& points);

    const QVector2D& min() const { return _min; }
    const QVector2D& max() const { return _max; }

    float xLength() const { return _max.x() - _min.x(); }
    float yLength() const { return _max.y() - _min.y(); }
    float maxLength() const;

    QVector2D xVector() const { return QVector2D(xLength(), 0.0f); }
    QVector2D yVector() const { return QVector2D(0.0f, yLength()); }

    float area() const { return xLength() * yLength(); }

    void set(const QVector2D& min, const QVector2D& max);
    void expandToInclude(const QVector2D& point);
    void expandToInclude(const BoundingBox2D& other);

    bool containsPoint(const QVector2D& point) const;
    bool containsLine(const Line2D& line) const;

    QVector2D centre() const;

    BoundingBox2D operator+(const QVector2D v) const { return BoundingBox2D(_min + v, _max + v); }
    BoundingBox2D operator*(float s) const { return BoundingBox2D(_min * s, _max * s); }
};

class Ray;

class BoundingBox3D
{
private:
    QVector3D _min;
    QVector3D _max;

public:
    BoundingBox3D();
    BoundingBox3D(QVector3D min, QVector3D max);
    explicit BoundingBox3D(const std::vector<QVector3D>& points);

    const QVector3D& min() const { return _min; }
    const QVector3D& max() const { return _max; }

    float xLength() const { return _max.x() - _min.x(); }
    float yLength() const { return _max.y() - _min.y(); }
    float zLength() const { return _max.z() - _min.z(); }
    float maxLength() const;

    QVector3D xVector() const { return QVector3D(xLength(), 0.0f, 0.0f); }
    QVector3D yVector() const { return QVector3D(0.0f, yLength(), 0.0f); }
    QVector3D zVector() const { return QVector3D(0.0f, 0.0f, zLength()); }

    void scale(float s);
    BoundingBox3D scaled(float s) const;

    float volume() const { return xLength() * yLength() * zLength(); }

    void set(const QVector3D& min, const QVector3D& max);
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const BoundingBox3D& other);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const Line3D& line) const;
    bool intersects(const Ray& ray, float t0, float t1) const;
    bool intersects(const Ray& ray) const;

    QVector3D centre() const;

    bool valid() const;

    BoundingBox3D operator+(const QVector3D v) const { return BoundingBox3D(_min + v, _max + v); }
    BoundingBox3D operator*(float s) const { return BoundingBox3D(_min * s, _max * s); }
};

#endif // BOUNDINGBOX_H
