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

#include "colorpalette.h"

#include "shared/utils/utils.h"
#include "shared/utils/container.h"
#include "shared/utils/color.h"

#include <json_helper.h>

#include <QRegularExpression>
#include <QDebug>

#include <algorithm>

ColorPalette::ColorPalette(const QString& descriptor)
{
    auto jsonDocument = parseJsonFrom(descriptor.toUtf8());

    if(jsonDocument.is_null())
    {
        qDebug() << "ColorPalette failed to parse" << descriptor;
        return;
    }

    if(!jsonDocument.is_object())
    {
        qDebug() << "ColorPalette is not an object" << descriptor;
        return;
    }

    const bool hasAutoColors = u::contains(jsonDocument, "autoColors");
    const bool hasFixedColors = u::contains(jsonDocument, "fixedColors");

    if(!hasAutoColors && !hasFixedColors)
    {
        qDebug() << "ColorPalette does not contain autoColors or fixedColors";
        return;
    }

    auto autoColorsValue = jsonDocument["autoColors"];
    auto fixedColorsValue = jsonDocument["fixedColors"];

    if(!autoColorsValue.is_array() && !fixedColorsValue.is_object())
    {
        qDebug() << "ColorPalette does not have autoColors array or fixedColors object";
        return;
    }

    if(hasAutoColors)
    {
        for(const auto& color : autoColorsValue)
        {
            auto colorString = QString::fromStdString(color);
            _colors.emplace_back(colorString);
        }
    }

    if(hasFixedColors)
    {
        for(const auto& i : fixedColorsValue.items())
        {
            auto value = QString::fromStdString(i.key());
            auto colorString = QString::fromStdString(i.value());

            _fixedColors[value] = colorString;
        }
    }

    if(u::contains(jsonDocument, "defaultColor"))
    {
        auto defaultColorValue = jsonDocument["defaultColor"];

        if(!defaultColorValue.is_string())
        {
            qDebug() << "ColorPalette.defaultColor is not a string";
            return;
        }

        auto defaultColorString = QString::fromStdString(defaultColorValue);
        _defaultColor = QColor(defaultColorString);
    }
}

QColor ColorPalette::get(const QString& value, int index) const
{
    if(u::contains(_fixedColors, value))
    {
        // Fixed colors always take precedence
        auto fixedColor = _fixedColors.at(value);
        return fixedColor;
    }

    if(index < 0)
    {
        // No index available, so derive one from the value itself

        QString nonDigitValue;
        index = 0;

        // Sum up all the sections of digits in the value
        static const QRegularExpression re(QStringLiteral(R"(([^\d]*)(\d*)([^\d]*))"));
        auto i = re.globalMatch(value);
        while(i.hasNext())
        {
            auto m = i.next();
            if(m.hasMatch())
            {
                auto prefix = m.captured(1);
                auto digits = m.captured(2);
                auto postfix = m.captured(3);

                nonDigitValue += prefix + postfix;

                if(!digits.isEmpty())
                {
                    bool success = false;
                    auto n = digits.toInt(&success);

                    if(success)
                        index += n;
                }
            }
        }

        // Add the unicode values of each non-digit character to the total
        for(const auto& c : std::as_const(nonDigitValue))
            index += c.unicode();
    }

    if(!_colors.empty())
    {
        auto colorIndex = static_cast<size_t>(index) % _colors.size();
        auto color = _colors.at(colorIndex);
        auto h = color.hue();
        auto s = color.saturation();
        auto v = color.value();

        auto hueIndex = index / static_cast<int>(_colors.size());
        if(hueIndex > 0)
        {
            if(_defaultColor.isValid())
                return _defaultColor;

            // If the base color has low saturation or
            // low value, adjust these before touching the hue
            if(s < 128 && (hueIndex > 1 || v >= 128))
            {
                hueIndex--;
                s += 128;
            }

            if(v < 128)
            {
                hueIndex--;
                v += 128;
            }

            // Rotate the hue around the base hue
            const int hRange = 90;
            int hValue = (hueIndex * 31) % hRange;
            if(hValue > (hRange / 2))
                hValue -= hRange;
            hValue += 360;

            h = (h + hValue) % 360;
        }

        return QColor::fromHsv(h, s, v);
    }

    if(_defaultColor.isValid())
        return _defaultColor;

    return u::colorForString(value);
}
