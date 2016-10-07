#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QString>

#include <vector>
#include <limits>

class Attribute
{
public:
    enum class Type
    {
        Unknown,
        String,
        Integer,
        Float
    };

private:
    Type _type = Type::Unknown;

    QString _name;

    std::vector<QString> _values;

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::min();
    double _floatMin = std::numeric_limits<double>::max();
    double _floatMax = std::numeric_limits<double>::min();

public:
    Attribute() = default;
    Attribute(const Attribute&) = default;
    Attribute(Attribute&&) = default;

    Attribute(const QString& name) :
        _name(name)
    {}

    Type type() const { return _type; }
    const QString& name() const { return _name; }
    int size() const { return static_cast<int>(_values.size()); }
    void reserve(int size) { _values.reserve(size); }

    int intMin() const { return _intMin; }
    int intMax() const { return _intMax; }
    int floatMin() const { return _floatMin; }
    int floatMax() const { return _floatMax; }

    void set(int index, const QString& value);
    QString get(int index) const;
};

#endif // ATTRIBUTE_H
