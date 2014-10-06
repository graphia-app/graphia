#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <algorithm>
#include <array>

template<typename T, size_t Size> class CircularBuffer
{
private:
    std::array<T, Size> _array;
    size_t _size;
    size_t _current;
    size_t _next;

public:
    CircularBuffer() :
        _size(0), _current(0), _next(0)
    {}

    void push_back(const T& t)
    {
        _array[_next] = t;
        _current = _next;
        _next = (_next + 1) % Size;
        _size = std::max(_size, _next);
    }

    T& at(int index) { return _array[(_current + Size + index) % Size]; }
    const T& at(int index) const { return _array[(_current + Size + index) % Size]; }

    T& front() { return at(0); }
    const T& front() const { return at(0); }

    size_t size() const { return _size; }
};

#endif // CIRCULARBUFFER_H
