#ifndef ITERATOR_RANGE_H
#define ITERATOR_RANGE_H

template<typename It> class iterator_range
{
public:
    iterator_range(It begin, It end) :
        _begin(begin), _end(end) {}

    It begin() const { return _begin; }
    It end() const { return _end; }

private:
    It _begin;
    It _end;
};

template<typename It>
iterator_range<It> make_iterator_range(It begin, It end)
{
    return iterator_range<It>(begin, end);
}

#endif // ITERATOR_RANGE_H
