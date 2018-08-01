#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "thirdparty/gccdiagaware.h"

#ifdef GCC_DIAGNOSTIC_AWARE
#pragma GCC diagnostic push

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wlogical-op"
#endif
#endif

#ifdef _MSC_VER
// MSVC warnings
#pragma warning( push )
#pragma warning( disable : 28020 ) // The expression ... is not true at this call
#endif

#include "json.hpp"

#ifdef GCC_DIAGNOSTIC_AWARE
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif

using json = nlohmann::json;

#include "shared/loading/progressfn.h"
#include "shared/loading/progress_iterator.h"

#include <QString>
#include <QByteArray>

inline void to_json(json& j, const QString& s)
{
    j = s.toStdString();
}

inline void from_json(const json& j, QString& s)
{
    s = QString::fromStdString(j.get<std::string>());
}

template<typename C>
json jsonArrayFrom(const C& container, ProgressFn progressFn = [](int){})
{
    json array;

    uint64_t i = 0;
    for(const auto& value : container)
    {
        array.emplace_back(value);
        progressFn(static_cast<int>((i++) * 100 / container.size()));
    }

    progressFn(-1);

    return array;
}

template<typename ProgressFn, typename CancelledFn>
json parseJsonFrom(const QByteArray& byteArray, const ProgressFn& progressFn, const CancelledFn& cancelledFn)
{
    json result;

    using JSONByteArrayIterator = progress_iterator<QByteArray::const_iterator>;
    JSONByteArrayIterator it(byteArray.begin());
    JSONByteArrayIterator end(byteArray.end());

    it.onPositionChanged(
    [&byteArray, &progressFn](size_t position)
    {
        progressFn(static_cast<int>((position * 100) / byteArray.size()));
    });

    it.setCancelledFn(cancelledFn);

    try
    {
        result = json::parse(it, end, nullptr, false);
    }
    catch(JSONByteArrayIterator::cancelled_exception&) {}

    return result;
}

#endif // JSON_HELPER_H
