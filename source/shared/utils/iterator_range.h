#ifndef ITERATOR_RANGE_H
#define ITERATOR_RANGE_H

template<typename BeginIt, typename EndIt> class iterator_range
{
public:
    iterator_range(BeginIt begin, EndIt end) :
        _begin(begin), _end(end) {}

    BeginIt& begin() { return _begin; }
    EndIt& end() { return _end; }

private:
    BeginIt _begin;
    EndIt _end;
};

template<typename BeginIt, typename EndIt>
auto make_iterator_range(BeginIt&& begin, EndIt&& end)
{
    return iterator_range<BeginIt, EndIt>(begin, end);
}

#endif // ITERATOR_RANGE_H
