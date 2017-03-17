#ifndef FLAGS_H
#define FLAGS_H

template<typename Enum>
class Flags
{
private:
    Enum _value;

public:
    Flags() = default;
    Flags(Enum value) :
        _value(value)
    {}

    void set(Enum value)
    {
        _value = static_cast<Enum>(static_cast<int>(_value) | static_cast<int>(value));
    }

    template<typename... Tail>
    void set(Enum value, Tail... values)
    {
        set(value);
        set(values...);
    }

    void reset(Enum value)
    {
        _value = static_cast<Enum>(static_cast<int>(_value) & ~static_cast<int>(value));
    }

    template<typename... Tail>
    void reset(Enum value, Tail... values)
    {
        reset(value);
        reset(values...);
    }

    bool test(Enum value) const
    {
        return static_cast<int>(_value) & static_cast<int>(value);
    }

    bool anyOf(Enum value) const { return test(value); }
    template<typename... Tail>
    bool anyOf(Enum value, Tail... values) const
    {
        return test(value) || anyOf(values...);
    }

    bool allOf(Enum value) const { return test(value); }
    template<typename... Tail>
    bool allOf(Enum value, Tail... values) const
    {
        return allOf(value) && allOf(values...);
    }

    Enum operator*() const { return _value; }
};

#endif // FLAGS_H
