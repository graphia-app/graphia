#ifndef BOOST_SPIRIT_QSTRING_ADAPTER_H
#define BOOST_SPIRIT_QSTRING_ADAPTER_H

#include <boost/spirit/home/x3/support/traits/container_traits.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

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

template<typename QStringIterator>
class QStringSpiritUnicodeIteratorAdaptor : public boost::iterator_adaptor<
    QStringSpiritUnicodeIteratorAdaptor<QStringIterator>,
    QStringIterator, char16_t, boost::forward_traversal_tag, char16_t>
{
public:
    using QStringSpiritUnicodeIteratorAdaptor::iterator_adaptor_::iterator_adaptor_;

private:
    friend class boost::iterator_core_access;
    char16_t dereference() const { return static_cast<char16_t>(this->base_reference()->unicode()); }
};

using QStringSpiritUnicodeIterator = QStringSpiritUnicodeIteratorAdaptor<QString::iterator>;
using QStringSpiritUnicodeConstIterator = QStringSpiritUnicodeIteratorAdaptor<QString::const_iterator>;

#endif // BOOST_SPIRIT_QSTRING_ADAPTER_H
