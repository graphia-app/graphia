#ifndef STRING_H
#define STRING_H

#include <QString>
#include <string>
#include <vector>
#include <istream>

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
} // namespace u
#endif // STRING_H
