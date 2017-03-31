#ifndef IATTRIBUTERANGE_H
#define IATTRIBUTERANGE_H

class IAttribute;

template<typename T>
class IAttributeRange
{
public:
    virtual ~IAttributeRange() = default;

    virtual bool hasMin() const = 0;
    virtual bool hasMax() const = 0;
    virtual bool hasRange() const = 0;

    virtual T min() const = 0;
    virtual T max() const = 0;
    virtual IAttribute& setMin(T) { Q_ASSERT(!"setMin not implemented!"); }
    virtual IAttribute& setMax(T) { Q_ASSERT(!"setMax not implemented!"); }

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
