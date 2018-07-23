#include "color.h"

#include <cmath>

#include <QCryptographicHash>

QColor u::contrastingColor(const QColor& color)
{
    auto brightness = 0.299 * color.redF() +
                      0.587 * color.greenF() +
                      0.114 * color.blueF();
    auto blackDiff = std::abs(brightness - 0.0);
    auto whiteDiff = std::abs(brightness - 1.0);

    return (blackDiff > whiteDiff) ? Qt::black : Qt::white;
}

QColor u::colorForString(const QString& string)
{
    QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
    hash.addData(string.toUtf8());
    auto result = hash.result();
    int hue = 0, lightness = 0;

    for(int i = 0; i < result.size(); i++)
    {
        int byte = result.at(i) + 128;
        if(i < result.size() / 2)
            hue = (hue + byte) % 255;
        else
            lightness = (lightness + byte) % 127;
    }

    lightness += 64;

    return QColor::fromHsl(hue, 255, lightness);
}
