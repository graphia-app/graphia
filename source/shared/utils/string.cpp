#include "string.h" //NOLINT

#include <QStringList>
#include <QRegularExpression>
#include <QLocale>

#include <vector>
#include <cmath>

bool u::isNumeric(const std::string& string)
{
    std::size_t pos;
    long double value = 0.0;

    try
    {
        value = std::stold(string, &pos);
    }
    catch(std::invalid_argument&)
    {
        return false;
    }
    catch(std::out_of_range&)
    {
        return false;
    }

    return pos == string.size() && !std::isnan(value);
}

bool u::isNumeric(const QString& string)
{
    bool success = false;

    // cppcheck-suppress ignoredReturnValue
    string.toDouble(&success);

    return success;
}

std::vector<QString> u::toQStringVector(const QStringList& stringList)
{
    std::vector<QString> v;
    v.reserve(stringList.size());

    for(const auto& string : stringList)
        v.emplace_back(string);

    return v;
}

QStringList u::toQStringList(const std::vector<QString>& qStringVector)
{
    QStringList l;
    l.reserve(static_cast<int>(qStringVector.size()));

    for(const auto& string : qStringVector)
        l.append(string);

    return l;
}

// https://stackoverflow.com/a/6089413/2721809
std::istream& u::getline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    Q_UNUSED(se);
    std::streambuf* sb = is.rdbuf();

    while(true)
    {
        int c = sb->sbumpc();
        switch(c)
        {
        case '\n':
            return is;

        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;

        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;

        default:
            t += static_cast<char>(c);
        }
    }
}

static QString stripZeroes(QString value)
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

QString u::formatNumberScientific(double value, int minDecimalPlaces, int maxDecimalPlaces,
                                  int minScientificFormattedStringDigitsThreshold,
                                  int maxScientificFormattedStringDigitsThreshold)
{
    QString formattedString;

    double smallThreshold = std::pow(10, -minScientificFormattedStringDigitsThreshold);
    double largeThreshold = std::pow(10, maxScientificFormattedStringDigitsThreshold);
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
            auto exponentValueString = stripZeroes(splitString[1]);

            exponentValueString.replace(QStringLiteral("-"), QStringLiteral(u"⁻"));
            exponentValueString.replace(QStringLiteral("+"), QString());

            for(auto& character : exponentValueString)
            {
                if(character.isDigit())
                    character = superScript[character.digitValue()];
            }

            formattedString = splitString[0] + QStringLiteral(u"×10") + exponentValueString;
        }
    }
    else
    {
        formattedString = QLocale::system().toString(value, 'f', maxDecimalPlaces);
        formattedString = stripZeroes(formattedString);
    }

    formattedString.replace(QStringLiteral("+"), QString());
    return formattedString;
}

QString u::formatNumberSIPostfix(double value)
{
    struct Postfix
    {
        double _threshold;
        double _divider;
        char _symbol;
    };

    std::vector<Postfix> postfixes =
    {
        {1e9, 1e9, 'B'},
        {1e6, 1e6, 'M'},
        {1e4, 1e3, 'k'},
    };

    for(const auto& postfix : postfixes)
    {
        if(value >= postfix._threshold)
        {
            auto d = value / postfix._divider;
            auto s = QString::number(d, 'f', 1);
            return s + postfix._symbol;
        }
    }

    return QString::number(value);
}
