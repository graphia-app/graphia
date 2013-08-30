#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QVector3D>

class BoundingBox
{
private:
    QVector3D _min;
    QVector3D _max;

public:
    BoundingBox();
    BoundingBox(const QVector3D& min, const QVector3D& max);

    const QVector3D& min() const { return _min; }
    const QVector3D& max() const { return _max; }

    float xLength() const { return _max.x() - _min.x(); }
    float yLength() const { return _max.y() - _min.y(); }
    float zLength() const { return _max.z() - _min.z(); }
    float maxLength() const;

    float volume() const { return xLength() * yLength() * zLength(); }

    void set(const QVector3D& min, const QVector3D& max);
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const BoundingBox& other);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const QVector3D& pointA, const QVector3D& pointB) const;

    QVector3D centre() const;
};

#endif // BOUNDINGBOX_H
