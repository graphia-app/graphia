#ifndef FIXEDSIZESTACK_H
#define FIXEDSIZESTACK_H

#include <array>

template<typename T> class FixedSizeStack
{
private:
    std::vector<T> _vector;
    size_t _size;
    int _top;

public:
    FixedSizeStack(size_t size) :
        _vector(size),
        _size(size),
        _top(-1)
    {}

    void push(const T& t)
    {
        if(_top + 1 < _size)
            _vector[++_top] = t;
    }

    T& top() { return _vector.at(_top); }
    const T& top() const { return _vector.at(_top); }

    T pop() { return _vector.at(_top--); }

    size_t size() const { return _size; }
    bool empty() const { return _top < 0; }
};

#endif // FIXEDSIZESTACK_H
