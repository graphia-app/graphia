#include "random.h"

#include <random>

static std::random_device randomh_rd;
static std::mt19937 randomh_mt19937(randomh_rd());

float u::rand(float low, float high)
{
    std::uniform_real_distribution<> distribution(low, high);
    return distribution(randomh_mt19937);
}

int u::rand(int low, int high)
{
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(randomh_mt19937);
}

QVector2D u::randQVector2D(float low, float high)
{
    return {rand(low, high), rand(low, high)};
}

QVector3D u::randQVector3D(float low, float high)
{
    return {rand(low, high), rand(low, high), rand(low, high)};
}

QColor u::randQColor()
{
    int r = rand(0, 255);
    int g = rand(0, 255);
    int b = rand(0, 255);

    return {r, g, b};
}
