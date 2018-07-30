#include "utils.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QLocale>

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

static void initQtResources()
{
    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(js);
}

QString u::stripZeroes(QString value)
{
    // Leading Zero
    auto indexOfDecimal = value.indexOf(QLocale::system().decimalPoint(), 1);
    for(int i = 0; i < value.length(); ++i)
    {
        if(!value[i].isDigit())
            continue;

        if(indexOfDecimal == 1)
            break;

        if(value[i] == '0')
            value.remove(i, 1);
        else
            break;
    }

    // Trailing Zero
    if(indexOfDecimal > -1)
    {
        while(indexOfDecimal < value.length() - 2)
        {
            if(value[value.length() - 1] == '0')
                value.chop(1);
            else
                break;
        }
    }
    return value;
}

QString u::formatNumberForDisplay(double value, int minDecimalPlaces, int maxDecimalPlaces,
                                  int minScientificformattedStringDigitsThreshold,
                                  int maxScientificformattedStringDigitsThreshold)
{
    QString formattedString;
    double smallThreshold = std::pow(10, -minScientificformattedStringDigitsThreshold);
    double largeThreshold = std::pow(10, maxScientificformattedStringDigitsThreshold);
    auto exponentChar = QLocale::system().exponential();

    if(maxDecimalPlaces == 0)
    {
        if(value <= 0.001)
            maxDecimalPlaces = 5;
        else if(value <= 0.01)
            maxDecimalPlaces = 4;
        else if(value <= 1.0)
            maxDecimalPlaces = 3;
        else if(value <= 100.0)
            maxDecimalPlaces = 2;
        else if(value <= 1000.0)
            maxDecimalPlaces = 1;
    }

    if(maxDecimalPlaces < minDecimalPlaces)
        maxDecimalPlaces = minDecimalPlaces;

    if(std::isfinite(value) &&
        ((value >= largeThreshold || value <= -largeThreshold) ||
        (value < smallThreshold && value > -smallThreshold && value != 0.0)))
    {
        formattedString = QLocale::system().toString(value, 'e', 2);
        auto splitString = formattedString.split(exponentChar);

        const QString superScript = QStringLiteral(u"⁰¹²³⁴⁵⁶⁷⁸⁹");
        if(splitString.length() > 0)
        {
            auto exponentValueString = u::stripZeroes(splitString[1]);

            for(int i = 0; i < exponentValueString.length(); ++i)
            {
                bool converted;
                int result = QString(exponentValueString[i]).toInt(&converted);
                if(converted)
                    exponentValueString[i] = superScript[result];
            }

            exponentValueString.replace(QStringLiteral("-"), QStringLiteral(u"⁻"));
            formattedString = splitString[0] + QStringLiteral(u"×10") + exponentValueString;
        }
    }
    else
    {
        formattedString = QLocale::system().toString(value, 'f', maxDecimalPlaces);
        formattedString = u::stripZeroes(formattedString);
    }

    formattedString.replace(QStringLiteral("+"), QString());
    return formattedString;
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
