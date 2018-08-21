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

#include "shared/loading/iparser.h"
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
json jsonArrayFrom(const C& container, Progressable* progressable = nullptr)
{
    json array;

    uint64_t i = 0;
    for(const auto& value : container)
    {
        array.emplace_back(value);

        if(progressable != nullptr)
            progressable->setProgress(static_cast<int>((i++) * 100 / container.size()));
    }

    if(progressable != nullptr)
        progressable->setProgress(-1);

    return array;
}

inline json parseJsonFrom(const QByteArray& byteArray, IParser& parser)
{
    json result;

    using JSONByteArrayIterator = progress_iterator<QByteArray::const_iterator>;
    JSONByteArrayIterator it(byteArray.begin());
    JSONByteArrayIterator end(byteArray.end());

    it.onPositionChanged(
    [&byteArray, &parser](size_t position)
    {
        parser.setProgress(static_cast<int>((position * 100) / byteArray.size()));
    });

    it.setCancelledFn([&parser] { return parser.cancelled(); });

    try
    {
        result = json::parse(it, end, nullptr, false);
    }
    catch(JSONByteArrayIterator::cancelled_exception&) {}

    return result;
}

#endif // JSON_HELPER_H
