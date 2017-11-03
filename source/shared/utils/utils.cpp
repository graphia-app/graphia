#include "utils.h"

#include <QCoreApplication>

int u::smallestPowerOf2GreaterThan(int x)
{
    if(x < 0)
        return 0;

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

static void initQtResources()
{
    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(js);
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
