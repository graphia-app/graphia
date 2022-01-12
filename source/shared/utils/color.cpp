/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
