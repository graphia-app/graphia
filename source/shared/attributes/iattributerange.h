#ifndef IATTRIBUTERANGE_H
#define IATTRIBUTERANGE_H

// In case windows.h has been included prior to this file
#undef min
#undef max

class IAttribute;

template<typename T>
class IAttributeRange
{
public:
    IAttributeRange() = default;
    IAttributeRange(const IAttributeRange&) = default;
    IAttributeRange(IAttributeRange&&) = default;
    IAttributeRange& operator=(const IAttributeRange&) = default;
    IAttributeRange& operator=(IAttributeRange&&) = default;

    virtual ~IAttributeRange() = default;

    virtual bool hasMin() const = 0;
    virtual bool hasMax() const = 0;
    virtual bool hasRange() const = 0;

    virtual T min() const = 0;
    virtual T max() const = 0;
    virtual IAttribute& setMin(T) = 0;
    virtual IAttribute& setMax(T) = 0;

    bool valueInRange(T value) const
    {
        if(hasMin() && value < min())
            return false;

        if(hasMax() && value > max())
            return false;

        return true;
    }
};

#endif // IATTRIBUTERANGE_H
