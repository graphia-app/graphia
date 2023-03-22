#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#ifdef _MSC_VER
// MSVC warnings
#pragma warning( push )
#pragma warning( disable : 28020 ) // The expression ... is not true at this call
#endif

#include <json.hpp>

using json = nlohmann::json;

#include "shared/loading/iparser.h"
#include "shared/graph/elementid.h"
#include "shared/utils/progressable.h"
#include "shared/loading/progress_iterator.h"

#include <QString>
#include <QUrl>
#include <QFile>
#include <QByteArray>

inline void to_json(json& j, const QString& s)
{
    j = s.toStdString();
}

inline void from_json(const json& j, QString& s)
{
    s = QString::fromStdString(j.get<std::string>());
}

inline void from_json(const json& j, QUrl& url)
{
    url = QString::fromStdString(j.get<std::string>());
}

template<typename E>
void to_json(json& j, ElementId<E> elementId)
{
    j = static_cast<int>(elementId);
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

inline json parseJsonFrom(const QByteArray& byteArray, IParser* parser = nullptr)
{
    json result;

    using JSONByteArrayIterator = progress_iterator<QByteArray::const_iterator>;
    JSONByteArrayIterator it(byteArray.begin());
    JSONByteArrayIterator end(byteArray.end());

    if(parser != nullptr)
    {
        it.onPositionChanged(
        [&byteArray, &parser](size_t position)
        {
            parser->setProgress(static_cast<int>((position * 100) / byteArray.size()));
        });

        it.setCancelledFn([&parser] { return parser->cancelled(); });
    }

    try
    {
        result = json::parse(it, end);
    }
    catch(json::parse_error& e)
    {
        if(parser != nullptr)
            parser->setFailureReason(QString::fromStdString(e.what()));
    }
    catch(JSONByteArrayIterator::cancelled_exception&) {}

    return result;
}

inline json parseJsonFrom(const QString& filename)
{
    QFile file(filename);
    if(!file.open(QIODeviceBase::ReadOnly))
        return {};

    return parseJsonFrom(file.readAll());
}

#endif // JSON_HELPER_H
