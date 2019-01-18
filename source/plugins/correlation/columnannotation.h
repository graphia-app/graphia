#ifndef COLUMNANNOTATION_H
#define COLUMNANNOTATION_H

#include <QString>

#include <vector>
#include <set>

class ColumnAnnotation
{
private:
    QString _name;

    std::vector<QString> _values;
    std::set<QString> _uniqueValues;

public:
    using Iterator = std::vector<QString>::const_iterator;

    ColumnAnnotation(QString name, std::vector<QString> values);
    ColumnAnnotation(QString name, const Iterator& begin, const Iterator& end);

    const QString& name() const { return _name; }
    const std::set<QString> uniqueValues() const { return _uniqueValues; }

    const QString& valueAt(size_t index) const { return _values.at(index); }
};

#endif // COLUMNANNOTATION_H
