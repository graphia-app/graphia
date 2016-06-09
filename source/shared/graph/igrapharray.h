#ifndef IGRAPHARRAY_H
#define IGRAPHARRAY_H

// This is the required interface from the application's point
// of view; how it actually stores the data is not important
class IGraphArray
{
public:
    virtual ~IGraphArray() = default;

    virtual void resize(int size) = 0;
    virtual void invalidate() = 0;
};

#endif // IGRAPHARRAY_H
