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

    constexpr bool static_strcmp(char const* a, char const* b)
    {
        return (*a && *b) ? (*a == *b && static_strcmp(a + 1, b + 1)) : (!*a && !*b);
    }

    std::vector<QString> toQStringVector(const QStringList& stringList);

    std::istream& getline(std::istream& is, std::string& t);
}
#endif // STRING_H
