#ifndef TYPEIDENTITY_H
#define TYPEIDENTITY_H

class QString;

class TypeIdentity
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

public:
    void updateType(const QString& value);

    template<typename C>
    void updateType(const C& values)
    {
        for(const auto& value : values)
            update(value);
    }

    void setType(Type type) { _type = type; }
    Type type() const { return _type; }
};

#endif // TYPEIDENTITY_H
