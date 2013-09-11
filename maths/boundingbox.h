#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QVector2D>
#include <QVector3D>

class BoundingBox2D
{
private:
    QVector2D _min;
    QVector2D _max;

public:
    BoundingBox2D();
    BoundingBox2D(const QVector2D& min, const QVector2D& max);

    const QVector2D& min() const { return _min; }
    const QVector2D& max() const { return _max; }

    float xLength() const { return _max.x() - _min.x(); }
    float yLength() const { return _max.y() - _min.y(); }
    float maxLength() const;

    float area() const { return xLength() * yLength(); }

    void set(const QVector2D& min, const QVector2D& max);
    void expandToInclude(const QVector2D& point);
    void expandToInclude(const BoundingBox2D& other);

    bool containsPoint(const QVector2D& point) const;
    bool containsLine(const QVector2D& pointA, const QVector2D& pointB) const;

    QVector2D centre() const;
};

class BoundingBox3D
{
private:
    QVector3D _min;
    QVector3D _max;

public:
    BoundingBox3D();
    BoundingBox3D(const QVector3D& min, const QVector3D& max);

    const QVector3D& min() const { return _min; }
    const QVector3D& max() const { return _max; }

    float xLength() const { return _max.x() - _min.x(); }
    float yLength() const { return _max.y() - _min.y(); }
    float zLength() const { return _max.z() - _min.z(); }
    float maxLength() const;

    float volume() const { return xLength() * yLength() * zLength(); }

    void set(const QVector3D& min, const QVector3D& max);
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const BoundingBox3D& other);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const QVector3D& pointA, const QVector3D& pointB) const;

    QVector3D centre() const;
};

#endif // BOUNDINGBOX_H
