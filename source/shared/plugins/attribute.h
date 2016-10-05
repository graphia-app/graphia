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

    Attribute(const QString& name, int numRows) :
        _name(name), _values(numRows)
    {}

    Type type() const { return _type; }
    const QString& name() const { return _name; }

    int intMin() const { return _intMin; }
    int intMax() const { return _intMax; }
    int floatMin() const { return _floatMin; }
    int floatMax() const { return _floatMax; }

    void set(int index, const QString& value);
    const QString& get(int index) const;
};

class Attributes
{
private:
    using Vector = std::vector<Attribute>;

    Vector _attributes;
    int _size = 0;

    Attribute& attributeByName(const QString& name);
    const Attribute& attributeByName(const QString& name) const;

public:
    int size() const { return _size; }
    void setSize(int size) { _size = size; }

    bool empty() const { return _attributes.empty(); }

    Vector::const_iterator begin() const { return _attributes.begin(); }
    Vector::const_iterator end() const { return _attributes.end(); }

    const QString& firstAttributeName() const { return _attributes.front().name(); }

    void add(const QString& name);
    void setValue(int index, const QString& name, const QString& value);
    const QString& value(int index, const QString& name) const;
};

#endif // ATTRIBUTE_H
