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

QString u::formatNumberForDisplay(double value, int minDecimalPlaces, int maxDecimalPlaces, int minScientificformattedStringDigitsThreshold, int maxScientificformattedStringDigitsThreshold)
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

    if(value >= largeThreshold || value <= -largeThreshold ||
            (value < smallThreshold && value > -smallThreshold && value != 0.0))
    {
        formattedString = QLocale::system().toString(value, 'e', 2);
        auto splitString = formattedString.split(exponentChar);
        if(splitString.length() > 1)
            formattedString = splitString[0] + exponentChar + u::stripZeroes(splitString[1]);
    }
    else
    {
        formattedString = QLocale::system().toString(value, 'f', maxDecimalPlaces);
        formattedString = u::stripZeroes(formattedString);
    }

    const QString superScript = "⁰¹²³⁴⁵⁶⁷⁸⁹";
    int startExponent = formattedString.indexOf(exponentChar);
    if(startExponent != -1)
    {
        for(int i = formattedString.indexOf(exponentChar) + 1; i < formattedString.length(); ++i)
        {
            bool converted;
            int result = QString(formattedString[i]).toInt(&converted);
            if(converted)
                formattedString[i] = superScript[result];
        }
    }
    formattedString.replace(exponentChar, "×10");
    // Replace - sign with superscript, but ignore first char which may be - if minus
    formattedString.replace(formattedString.indexOf("-", 1), 1, "⁻");
    formattedString.replace("+", "");
    return formattedString;
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
