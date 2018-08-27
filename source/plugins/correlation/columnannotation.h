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
    ColumnAnnotation(QString name, std::vector<QString> values);

    const QString& name() const { return _name; }
    const std::set<QString> uniqueValues() const { return _uniqueValues; }

    const QString& valueAt(size_t index) const { return _values.at(index); }
};

#endif // COLUMNANNOTATION_H
