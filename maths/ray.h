#ifndef RAY_H
#define RAY_H

#include <QVector3D>

class Ray
{
private:
    QVector3D _origin;
    QVector3D _dir;
    QVector3D _invDir;
    int _sign[3];

public:
    Ray(const QVector3D& _origin, const QVector3D& _dir) :
        _origin(_origin), _dir(_dir)
    {
        _invDir = QVector3D(1.0f / _dir.x(), 1.0f / _dir.y(), 1.0f/ _dir.z());
        _sign[0] = (_invDir.x() < 0.0f);
        _sign[1] = (_invDir.y() < 0.0f);
        _sign[2] = (_invDir.z() < 0.0f);
    }

    const QVector3D& origin() const { return _origin; }
    const QVector3D& dir() const { return _dir; }
    const QVector3D& invDir() const { return _invDir; }
    const int* sign() const { return _sign; }

    QVector3D closestPointTo(const QVector3D& point) const;
    QVector3D closestPointTo(const Ray& other) const;
};

#endif // RAY_H
