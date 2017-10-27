#include "random.h"

#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());

float u::rand(float low, float high)
{
    std::uniform_real_distribution<> distribution(low, high);
    return distribution(gen);
}

int u::rand(int low, int high)
{
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(gen);
}

QVector2D u::randQVector2D(float low, float high)
{
    return QVector2D(rand(low, high), rand(low, high));
}

QVector3D u::randQVector3D(float low, float high)
{
    return QVector3D(rand(low, high), rand(low, high), rand(low, high));
}

QColor u::randQColor()
{
    int r = rand(0, 255);
    int g = rand(0, 255);
    int b = rand(0, 255);

    return QColor(r, g, b);
}
