/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "string.h" //NOLINT

#include <QStringView>
#include <QStringList>
#include <QRegularExpression>
#include <QLocale>

#include <vector>
#include <cmath>
#include <sstream>
#include <limits>

using namespace Qt::Literals::StringLiterals;

bool u::isNumeric(const std::string& string)
{
    if(string.empty())
        return false;

    std::stringstream ss;
    ss << string;

    long double value = 0.0;
    ss >> value;

    return ss.eof();
}

bool u::isNumeric(const QString& string)
{
    bool success = false;

    string.toDouble(&success);

    return success;
}

bool u::isInteger(const std::string& string)
{
    if(string.empty())
        return false;

    std::stringstream ss;
    ss << string;

    long int value = 0;
    ss >> value;

    return ss.eof();
}

bool u::isInteger(const QString& string)
{
    bool success = false;

    string.toInt(&success);

    return success;
}

double u::toNumber(const std::string& string)
{
    if(string.empty())
        return std::numeric_limits<double>::quiet_NaN();

    std::stringstream ss;
    ss << string;

    double value = 0.0;
    ss >> value;

    if(ss.eof())
        return value;

    return std::numeric_limits<double>::quiet_NaN();
}

double u::toNumber(const QString& string)
{
    bool success = false;
    auto value = string.toDouble(&success);

    if(success)
        return value;

    return std::numeric_limits<double>::quiet_NaN();
}

std::vector<QString> u::toQStringVector(const QStringList& stringList)
{
    std::vector<QString> v;
    v.reserve(static_cast<size_t>(stringList.size()));
    std::copy(stringList.begin(), stringList.end(), std::back_inserter(v));
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

    const std::istream::sentry se(is, true);
    Q_UNUSED(se);
    std::streambuf* sb = is.rdbuf();

    while(true)
    {
        const int c = sb->sbumpc();
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
    auto indexOfDecimalPoint = value.indexOf(QLocale::system().decimalPoint(), 1);
    auto indexOfLastDigit = (indexOfDecimalPoint < 0 ? value.length() : indexOfDecimalPoint) - 1;

    int i = 0;

    // Skip leading non-digits (+/- etc.)
    while(i < (indexOfLastDigit + 1) && !value[i].isDigit())
        i++;

    // Strip leading zeroes
    while(i < indexOfLastDigit && value[i].isDigit() && value[i] == '0')
    {
        value.remove(i, 1);
        indexOfDecimalPoint--;
        indexOfLastDigit--;
    }

    if(indexOfDecimalPoint >= 0)
    {
        // Strip trailing zeroes
        while(value[value.length() - 1] == '0')
            value.chop(1);

        // Strip trailing decimal point
        if(value.length() - 1 == indexOfDecimalPoint)
            value.chop(1);
    }

    return value;
}

static int maxDecimalPlacesFor(double value)
{
    auto absValue = std::abs(value);

    if(absValue <= 0.001)   return 5;
    if(absValue <= 0.01)    return 4;
    if(absValue <= 1.0)     return 3;
    if(absValue <= 100.0)   return 2;
    if(absValue <= 1000.0)  return 1;

    return 0;
}

QString u::formatNumber(double value)
{
    QString formattedString = QLocale::system().toString(value, 'f', maxDecimalPlacesFor(value));
    formattedString = stripZeroes(formattedString);

    formattedString.replace(u"+"_s, u""_s);
    return formattedString;
}

QString u::formatNumberScientific(double value, int minDecimalPlaces, int maxDecimalPlaces,
                                  int minScientificFormattedStringDigitsThreshold,
                                  int maxScientificFormattedStringDigitsThreshold)
{
    QString formattedString;

    const double smallThreshold = std::pow(10, -minScientificFormattedStringDigitsThreshold);
    const double largeThreshold = std::pow(10, maxScientificFormattedStringDigitsThreshold);
    auto exponentChar = QLocale::system().exponential();
    auto absValue = std::abs(value);

    if(maxDecimalPlaces == 0)
        maxDecimalPlaces = maxDecimalPlacesFor(value);

    if(maxDecimalPlaces < minDecimalPlaces)
        maxDecimalPlaces = minDecimalPlaces;

    if(std::isfinite(value) && (absValue >= largeThreshold || (absValue < smallThreshold && value != 0.0)))
    {
        formattedString = QLocale::system().toString(value, 'e', 2);
        auto splitString = formattedString.split(exponentChar);

        const QString superScript = QStringLiteral(u"⁰¹²³⁴⁵⁶⁷⁸⁹");
        if(splitString.length() > 1)
        {
            auto exponentValueString = stripZeroes(splitString[1]);

            exponentValueString.replace(u"-"_s, u"⁻"_s);
            exponentValueString.replace(u"+"_s, u""_s);

            for(auto& character : exponentValueString)
            {
                if(character.isDigit())
                    character = superScript[character.digitValue()];
            }

            formattedString = splitString[0] + u"×10"_s + exponentValueString;
        }
    }
    else
    {
        formattedString = QLocale::system().toString(value, 'f', maxDecimalPlaces);
        formattedString = stripZeroes(formattedString);
    }

    formattedString.replace(u"+"_s, u""_s);
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

    const std::vector<Postfix> postfixes =
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

bool u::isHex(const std::string& string)
{
    return isHex(QString::fromStdString(string));
}

bool u::isHex(const QString& string)
{
    static const QRegularExpression re(u"^[a-fA-F0-9]+$"_s);
    return string.size() % 2 == 0 && re.match(string).hasMatch();
}

std::string u::hexToString(const std::string& string)
{
    std::string output;

    if(isHex(string))
    {
        for(size_t i = 0; i < string.length(); i += 2)
        {
            auto byteString = string.substr(i, 2);
            auto b = static_cast<char>(std::strtol(byteString.data(), nullptr, 16));
            output.push_back(b);
        }
    }

    return output;
}

QString u::hexToString(const QString& string)
{
    return QString::fromStdString(hexToString(string.toStdString()));
}

std::vector<std::byte> u::hexToBytes(const std::string& string)
{
    std::vector<std::byte> bytes;
    bytes.reserve(string.length() / 2);

    if(isHex(string))
    {
        for(size_t i = 0; i < string.length(); i += 2)
        {
            auto byteString = string.substr(i, 2);
            auto b = static_cast<std::byte>(std::strtol(byteString.data(), nullptr, 16));
            bytes.push_back(b);
        }
    }

    return bytes;
}

std::vector<std::byte> u::hexToBytes(const QString& string)
{
    return hexToBytes(string.toStdString());
}

QString u::escapeQuotes(QString s)
{
    s.replace(QStringLiteral(R"(")"), QStringLiteral(R"(\")"));
    return s;
}

QString u::pluralise(size_t count, const QString& singular, const QString& plural)
{
    if(count == 1)
        return u"1 %1"_s.arg(singular);

    return u"%1 %2"_s.arg(count).arg(plural);
}

static int numericCompare(const QString& a, const QString& b, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    auto tokenise = [](const QStringView& s)
    {
        std::vector<qsizetype> indices;

        for(qsizetype i = 1; i < s.size(); i++)
        {
            if(s[i].isDigit() != s[i - 1].isDigit())
                indices.push_back(i);
        }

        indices.push_back(s.size());

        return indices;
    };

    auto aTokens = tokenise(a);
    auto bTokens = tokenise(b);

    auto aIt = aTokens.begin();
    auto bIt = bTokens.begin();
    qsizetype aIndex = 0;
    qsizetype bIndex = 0;

    while(aIt != aTokens.end() && bIt != bTokens.end())
    {
        auto aToken = QStringView(a).sliced(aIndex, *aIt - aIndex);
        auto bToken = QStringView(b).sliced(bIndex, *bIt - bIndex);
        int diff = 0;

        if(aToken.front().isDigit() && bToken.front().isDigit())
        {
            auto aInt = aToken.toInt(nullptr);
            auto bInt = bToken.toInt(nullptr);
            diff = aInt - bInt;
        }
        else
            diff = aToken.compare(bToken, cs);

        if(diff != 0)
            return diff;

        aIndex = *aIt++;
        bIndex = *bIt++;
    }

    return a.compare(b, cs);
}

int u::numericCompare(const QString& a, const QString& b)
{
    return numericCompare(a, b, Qt::CaseSensitive);
}

int u::numericCompareCaseInsensitive(const QString& a, const QString& b)
{
    return numericCompare(a, b, Qt::CaseInsensitive);
}
