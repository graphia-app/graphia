#include "color.h"

#include <cmath>

QColor u::contrastingColor(const QColor& color)
{

    auto brightness = 0.299 * color.redF() +
                      0.587 * color.greenF() +
                      0.114 * color.blueF();
    auto blackDiff = std::abs(brightness - 0.0);
    auto whiteDiff = std::abs(brightness - 1.0);

    return (blackDiff > whiteDiff) ? Qt::black : Qt::white;
}
