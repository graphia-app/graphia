#ifndef BOOST_SPIRIT_QSTRING_ADAPTER_H
#define BOOST_SPIRIT_QSTRING_ADAPTER_H

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3/support/traits/container_traits.hpp"
#include "thirdparty/boost/boost_enable_warnings.h"

#include <QString>

namespace boost { namespace spirit { namespace x3 { namespace traits {

template<>
struct push_back_container<QString>
{
    template<typename T>
    static bool call(QString& c, T&& val)
    {
        c.push_back(std::move(val));
        return true;
    }
};

template<>
struct append_container<QString>
{
    template<typename Iterator>
    static bool call(QString& c, Iterator first, Iterator last)
    {
        c.append(first, std::distance(first, last));
        return true;
    }
};

template<>
struct is_empty_container<QString>
{
    static bool call(QString const& c)
    {
        return c.isEmpty();
    }
};

}}}}

#endif // BOOST_SPIRIT_QSTRING_ADAPTER_H
