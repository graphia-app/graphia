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
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

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

inline void from_json(const json& j, QVariant& v)
{
    switch(j.type())
    {
    default:
    case json::value_t::null:               v = {}; break;
    case json::value_t::boolean:            v = QVariant::fromValue(j.get<bool>()); break;
    case json::value_t::number_integer:     v = QVariant::fromValue(j.get<int>()); break;
    case json::value_t::number_unsigned:    v = QVariant::fromValue(j.get<unsigned int>()); break;
    case json::value_t::number_float:       v = QVariant::fromValue(j.get<float>()); break;
    case json::value_t::string:             v = QVariant::fromValue(QString::fromStdString(j.get<std::string>())); break;
    case json::value_t::array:
    {
        QVariantList variantList;

        for(const auto& element : j)
        {
            QVariant variantElement;
            from_json(element, variantElement);
            variantList.append(variantElement);
        }

        v = variantList;
        break;
    }
    case json::value_t::object:
    {
        QVariantMap variantMap;

        for(const auto& element : j.items())
        {
            auto key = QString::fromStdString(element.key());
            QVariant variantValue;
            from_json(element.value(), variantValue);
            variantMap.insert(key, variantValue);
        }

        v = QVariant::fromValue(variantMap);
        break;
    }
    }
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
