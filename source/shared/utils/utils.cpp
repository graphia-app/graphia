#include "utils.h"

#include <QCoreApplication>

int u::smallestPowerOf2GreaterThan(int x)
{
    if(x < 0)
        return 0;

    auto xu = static_cast<uint64_t>(x);
    xu--;
    xu |= xu >> 1;
    xu |= xu >> 2;
    xu |= xu >> 4;
    xu |= xu >> 8;
    xu |= xu >> 16;
    return static_cast<int>(xu + 1);
}

float u::normaliseAngle(float radians)
{
    while(radians > Constants::Pi())
        radians -= Constants::TwoPi();

    while(radians <= -Constants::Pi())
        radians += Constants::TwoPi();

    return radians;
}

static void initQtResources()
{
    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(js);
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
