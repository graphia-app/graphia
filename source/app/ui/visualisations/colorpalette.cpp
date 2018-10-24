#include "colorpalette.h"

#include "shared/utils/utils.h"

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
    auto baseColorsValue = jsonObject.value(QStringLiteral("baseColors"));

    if(!baseColorsValue.isArray())
    {
        qDebug() << "ColorPalette does not have baseColors array";
        return;
    }

    auto baseColorsArray = baseColorsValue.toArray();

    for(const auto& color : baseColorsArray)
    {
        auto colorString = color.toString();
        _colors.emplace_back(colorString);
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

QColor ColorPalette::get(const QString& value) const
{
    QString nonDigitValue;
    int digitValue = 0;

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
                    digitValue += n;
            }
        }
    }

    // Add the unicode values of each non-digit character to the total
    for(const auto c : qAsConst(nonDigitValue))
        digitValue += c.unicode();

    if(!_colors.empty())
    {
        auto index = digitValue % _colors.size();
        auto color = _colors.at(index);
        auto h = color.hue();
        auto s = color.saturation();
        auto v = color.value();

        auto hIndex = digitValue / _colors.size();
        if(hIndex > 0)
        {
            if(_otherColor.isValid())
                return _otherColor;

            // If the base color has low saturation or
            // low value, adjust these before touching the hue
            if(s < 128 && (hIndex > 1 || v >= 128))
            {
                hIndex--;
                s += 128;
            }

            if(v < 128)
            {
                hIndex--;
                v += 128;
            }

            // Rotate the hue around the base hue
            const int hRange = 90;
            int hValue = (hIndex * 31) % hRange;
            if(hValue > (hRange / 2))
                hValue -= hRange;
            hValue += 360;

            h = (h + hValue) % 360;
        }

        return QColor::fromHsv(h, s, v);
    }

    qWarning() << "ColorPalette is empty, using default color for" << value;
    return Qt::black;
}
