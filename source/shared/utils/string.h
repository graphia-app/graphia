/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef STRING_H
#define STRING_H

#include <QString>

#include <string>
#include <vector>
#include <istream>
#include <sstream>
#include <iomanip>
#include <cstddef>

class QStringList;

namespace u
{
    bool isNumeric(const std::string& string);
    bool isNumeric(const QString& string);

    double toNumber(const std::string& string);
    double toNumber(const QString& string);

    std::vector<QString> toQStringVector(const QStringList& stringList);
    QStringList toQStringList(const std::vector<QString>& qStringVector);

    std::istream& getline(std::istream& is, std::string& t);

    QString formatNumberScientific(double value, int minDecimalPlaces = 0, int maxDecimalPlaces = 0,
                                   int minScientificFormattedStringDigitsThreshold = 4,
                                   int maxScientificFormattedStringDigitsThreshold = 5);
    QString formatNumberSIPostfix(double value);

    bool isHex(const std::string& string);
    bool isHex(const QString& string);

    std::string hexToString(const std::string& string);
    QString hexToString(const QString& string);

    std::vector<std::byte> hexToBytes(const std::string& string);
    std::vector<std::byte> hexToBytes(const QString& string);

    template<typename T>
    std::string bytesToHex(const T& bytes)
    {
        std::ostringstream ss;

        ss << std::hex << std::setfill('0');
        for(int b : bytes)
        {
            if(b < 0)
                b += 0x100;

            ss << std::setw(2) << b;
        }

        return ss.str();
    }
} // namespace u
#endif // STRING_H
