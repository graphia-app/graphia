#include "utils.h"

#include <QCoreApplication>
#include <QRegularExpression>

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

QString u::formatNumberForDisplay(double value, int minDecimalPlaces, int maxDecimalPlaces, int minScientificformattedStringDigitsThreshold, int maxScientificformattedStringDigitsThreshold)
{
    QString formattedString;
    QRegularExpression trailingZeros(R"(^(-?(?:\d+\.\d+?[^0]*|\d+))0*$)");
    double smallThreshold = std::pow(10, -minScientificformattedStringDigitsThreshold);
    double largeThreshold = std::pow(10, maxScientificformattedStringDigitsThreshold);

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
            (value < smallThreshold && value > -smallThreshold && value != 0))
    {
        formattedString = QString::number(value, 'e', 2);
    }
    else
    {
        formattedString = QString::number(value, 'f', maxDecimalPlaces);
        formattedString = trailingZeros.match(formattedString).captured(1);
    }

    const QString superScript = "⁰¹²³⁴⁵⁶⁷⁸⁹";
    int startExponent = formattedString.indexOf("e");
    if(startExponent != -1)
    {
        for(int i = formattedString.indexOf("e") + 1; i < formattedString.length(); ++i)
        {
            bool converted;
            int result = QString(formattedString[i]).toInt(&converted);
            if(converted)
                formattedString[i] = superScript[result];
        }
    }
    formattedString.replace("e", "×10");
    // Replace - sign with superscript, but ignore first char which may be - if minus
    formattedString.replace(formattedString.indexOf("-", 1), 1, "⁻");
    formattedString.replace("+", "");
    return formattedString;
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
