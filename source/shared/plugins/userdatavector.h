#ifndef USERDATAVECTOR_H
#define USERDATAVECTOR_H

#include <QString>

#include "thirdparty/json/json_helper.h"

#include <vector>
#include <limits>
#include <utility>

class UserDataVector
{
public:
    enum class Type
    {
        Unknown,
        String,
        Int,
        Float
    };

private:
    Type _type = Type::Unknown;

    QString _name;

    std::vector<QString> _values;

    int _intMin = std::numeric_limits<int>::max();
    int _intMax = std::numeric_limits<int>::lowest();
    double _floatMin = std::numeric_limits<double>::max();
    double _floatMax = std::numeric_limits<double>::lowest();

public:
    UserDataVector() = default;
    UserDataVector(const UserDataVector&) = default;
    UserDataVector(UserDataVector&&) noexcept = default;

    explicit UserDataVector(QString name) :
        _name(std::move(name))
    {}

    auto begin() const { return _values.begin(); }
    auto end() const { return _values.end(); }

    Type type() const { return _type; }
    const QString& name() const { return _name; }
    int numValues() const { return static_cast<int>(_values.size()); }
    void reserve(int size) { _values.reserve(size); }

    int intMin() const { return _intMin; }
    int intMax() const { return _intMax; }
    double floatMin() const { return _floatMin; }
    double floatMax() const { return _floatMax; }

    void set(size_t index, const QString& value);
    QString get(size_t index) const;

    json save() const;
    bool load(const QString& name, const json& jsonObject);
};

#endif // USERDATAVECTOR_H
