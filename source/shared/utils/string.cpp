#include "string.h" //NOLINT

#include <QStringList>
#include <QRegularExpression>

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

std::vector<QString> u::toQStringVector(const QStringList& stringList)
{
    std::vector<QString> v(stringList.size());

    for(const auto& string : stringList)
        v.emplace_back(string);

    return v;
}

QStringList u::toQStringList(const std::vector<QString>& qStringVector)
{
    QStringList l;
    l.reserve(qStringVector.size());

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

QString u::formatUsingSIPostfix(double number)
{
    struct PostFix
    {
        double threshold;
        double divider;
        char symbol;
    };

    const std::vector<PostFix> si
    {
        {1E9, 1E9, 'B'},
        {1E6, 1E6, 'M'},
        {1E4, 1E3, 'k'}
    };

    for(const auto& v : si)
    {
        if(number >= v.threshold)
        {
            auto digits = QString::number(number / v.divider, 'f', 1);
            digits.replace(QRegularExpression(QStringLiteral(R"(\.0+$)")), QStringLiteral(""));

            return digits + v.symbol;
        }
    }

    return QString::number(number);
}
