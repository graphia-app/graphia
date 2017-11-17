#ifndef RANDOM_H
#define RANDOM_H

#include <QVector2D>
#include <QVector3D>
#include <QColor>

namespace u
{
    float rand(float low, float high);
    int rand(int low, int high);

    QVector2D randQVector2D(float low, float high);
    QVector3D randQVector3D(float low, float high);
    QColor randQColor();
} // namespace u

#endif // RANDOM_H
