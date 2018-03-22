#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "thirdparty/gccdiagaware.h"

#ifdef GCC_DIAGNOSTIC_AWARE
#pragma GCC diagnostic push

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wlogical-op"
#endif
#endif

#include "json.hpp"

#ifdef GCC_DIAGNOSTIC_AWARE
#pragma GCC diagnostic pop
#endif

using json = nlohmann::json;

#include "shared/loading/progressfn.h"

#include <QString>

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

#endif // JSON_HELPER_H
