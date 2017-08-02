#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <algorithm>
#include <array>

template<typename T, size_t Size> class CircularBuffer
{
private:
    std::array<T, Size> _array;
    size_t _size = 0;
    size_t _current = 0;
    size_t _next = 0;

public:
    void push_back(const T& t)
    {
        _array[_next] = t;
        _current = _next;
        _next++;
        _size = std::max(_size, _next);
        _next = _next % Size;
    }

    void fill(const T& t)
    {
        _array.fill(t);
        _size = Size;
        _current = 0;
        _next = 1;
    }

    // An index of 0 will get the oldest T
    // An index of size() - 1 will get the newest T
    const T& at(size_t index) const
    {
        Q_ASSERT(_size > 0);
        auto base = _current - _size + 1;
        return _array[(base + Size + index) % Size];
    }

    const T& newest() const
    {
        Q_ASSERT(_size > 0);
        return at(_size - 1);
    }

    const T& oldest() const
    {
        Q_ASSERT(_size > 0);
        return at(0);
    }

    T mean(size_t samples) const
    {
        samples = std::min(samples, _size);

        T result = T();
        float reciprocal = 1.0f / samples;

        for(auto i = _size - samples; i < _size; i++)
            result += at(i) * reciprocal;

        return result;
    }

    T mean() const { return mean(_size); }

    size_t size() const { return _size; }

    bool full() const { return _size >= Size; }

    void clear()
    {
        _size = 0;
        _next = 0;
        _current = 0;
    }
};

#endif // CIRCULARBUFFER_H
