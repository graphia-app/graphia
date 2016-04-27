#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"
#include "line.h"

#include <QVector3D>

class BaseFrustum
{
public:
    virtual ~BaseFrustum() {}

    virtual bool containsPoint(const QVector3D& point) const = 0;
    bool containsLine(const Line3D& line) const;

    virtual Line3D centreLine() const = 0;
};

class Frustum : public BaseFrustum
{
private:
    Plane _planes[6];
    Line3D _centreLine;

public:
    Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4);

    bool containsPoint(const QVector3D& point) const;
    Line3D centreLine() const { return _centreLine; }
};

#endif // FRUSTUM_H
