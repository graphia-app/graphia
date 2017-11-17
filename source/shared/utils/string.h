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

    std::vector<QString> toQStringVector(const QStringList& stringList);

    std::istream& getline(std::istream& is, std::string& t);
} // namespace u
#endif // STRING_H
