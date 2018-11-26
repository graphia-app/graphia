#include "colorpalette.h"

#include "shared/utils/utils.h"
#include "shared/utils/container.h"
#include "shared/utils/color.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>

#include <algorithm>

ColorPalette::ColorPalette(const QString& descriptor)
{
    QJsonParseError error{0, QJsonParseError::ParseError::NoError};
    auto jsonDocument = QJsonDocument::fromJson(descriptor.toUtf8(), &error);

    if(jsonDocument.isNull())
    {
        qDebug() << "ColorPalette failed to parse:" << error.errorString() << descriptor;
        return;
    }

    if(!jsonDocument.isObject())
    {
        qDebug() << "ColorPalette is not an object";
        return;
    }

    auto jsonObject = jsonDocument.object();
    auto autoColorsValue = jsonObject.value(QStringLiteral("autoColors"));
    auto fixedColorsValue = jsonObject.value(QStringLiteral("fixedColors"));

    if(!autoColorsValue.isArray() && !fixedColorsValue.isObject())
    {
        qDebug() << "ColorPalette does not have autoColors array or fixedColors object";
        return;
    }

    if(autoColorsValue.isArray())
    {
        auto autoColorsArray = autoColorsValue.toArray();

        for(const auto& color : autoColorsArray)
        {
            auto colorString = color.toString();
            _colors.emplace_back(colorString);
        }
    }

    if(fixedColorsValue.isObject())
    {
        auto fixedColorsObject = fixedColorsValue.toObject();

        for(const auto& key : fixedColorsObject.keys())
            _fixedColors[key] = fixedColorsObject.value(key).toString();
    }

    auto otherColorValue = jsonObject.value(QStringLiteral("otherColor"));

    if(otherColorValue.isUndefined())
        return;

    if(!otherColorValue.isString())
    {
        qDebug() << "ColorPalette.otherColor is not a string";
        return;
    }

    auto otherColorString = otherColorValue.toString();
    _otherColor = QColor(otherColorString);
}

QColor ColorPalette::get(const QString& value, const std::vector<QString>& values) const
{
    auto index = u::indexOf(values, value);

    if(u::contains(_fixedColors, value))
    {
        // Fixed colors always take precedence
        auto fixedColor = _fixedColors.at(value);
        return fixedColor;
    }
    else if(index < 0)
    {
        // No index available, so derive one from the value itself

        QString nonDigitValue;
        index = 0;

        // Sum up all the sections of digits in the value
        const QRegularExpression re(QStringLiteral(R"(([^\d]*)(\d*)([^\d]*))"));
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
                    bool success;
                    auto n = digits.toInt(&success);

                    if(success)
                        index += n;
                }
            }
        }

        // Add the unicode values of each non-digit character to the total
        for(const auto c : qAsConst(nonDigitValue))
            index += c.unicode();
    }

    if(!_colors.empty())
    {
        auto colorIndex = index % _colors.size();
        auto color = _colors.at(colorIndex);
        auto h = color.hue();
        auto s = color.saturation();
        auto v = color.value();

        auto hueIndex = index / _colors.size();
        if(hueIndex > 0)
        {
            if(_otherColor.isValid())
                return _otherColor;

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
    else if(_otherColor.isValid())
        return _otherColor;

    return u::colorForString(value);
}
