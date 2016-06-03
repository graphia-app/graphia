#ifndef IGRAPHARRAY_H
#define IGRAPHARRAY_H

class IGraphArray
{
public:
    virtual ~IGraphArray() = default;

    virtual void resize(int size) = 0;
    virtual void invalidate() = 0;
};

#endif // IGRAPHARRAY_H
