/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "legacynames.h"

#include "app/attributes/attribute.h"
#include "app/transform/graphtransform.h"
#include "app/transform/graphtransformconfigparser.h"
#include "app/transform/graphtransformparameter.h"
#include "app/transform/transforms/averageattributetransform.h"

#include "shared/utils/container.h"
#include "shared/utils/json.h"

#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <string>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace
{
struct QuotedString
{
    qsizetype _start = 0; // The opening quote
    qsizetype _end = 0; // One past the closing quote
    QString _content;
};
} // namespace

static std::vector<QuotedString> quotedStringsIn(const QString& text)
{
    std::vector<QuotedString> quotedStrings;
    qsizetype start = -1;

    for(qsizetype i = 0; i < text.size(); i++)
    {
        if(start >= 0 && text.at(i) == '\\')
        {
            // Skip the escaped character
            i++;
            continue;
        }

        if(text.at(i) != '"')
            continue;

        if(start < 0)
            start = i;
        else
        {
            quotedStrings.push_back({._start = start, ._end = i + 1,
                ._content = text.mid(start + 1, i - start - 1)});
            start = -1;
        }
    }

    return quotedStrings;
}

// Only quotes were escaped, so a backslash that doesn't escape a quote is
// itself escaped, now that a backslash can also escape another backslash
static QString upgradedEscapes(const QString& content)
{
    QString result;
    result.reserve(content.size());

    for(qsizetype i = 0; i < content.size(); i++)
    {
        if(content.at(i) == '\\' && i + 1 < content.size() && content.at(i + 1) == '"')
            result.append(content.at(i++));
        else if(content.at(i) == '\\')
            result.append('\\');

        result.append(content.at(i));
    }

    return result;
}

static QString renamed(const QString& text, const std::vector<QuotedString>& quotedStrings,
    const std::vector<QString>& newContents)
{
    auto result = text;

    // In reverse, so that the earlier positions remain valid
    for(auto i = quotedStrings.size(); i-- > 0;)
    {
        const auto& quotedString = quotedStrings.at(i);
        const auto newContent = upgradedEscapes(newContents.at(i));

        if(newContent == quotedString._content)
            continue;

        result.replace(quotedString._start, quotedString._end - quotedString._start,
            u"\"%1\""_s.arg(newContent));
    }

    return result;
}

LegacyNames::LegacyNames(TransformFactoryFn transformFactory, AttributeIsDeclaredFn attributeIsDeclared,
    std::set<QString> userAttributeNames) :
    _transformFactory(std::move(transformFactory)),
    _attributeIsDeclared(std::move(attributeIsDeclared)),
    _userAttributeNames(std::move(userAttributeNames))
{}

QString LegacyNames::transformName(const QString& name) const
{
    auto compactedName = compactName(name);
    return _transformFactory(compactedName) != nullptr ? compactedName : name;
}

QString LegacyNames::attributeName(const QString& name) const
{
    if(u::contains(_attributeNames, name))
        return _attributeNames.at(name);

    // Attributes from the user's data keep the names they had
    if(u::contains(_userAttributeNames, name))
        return name;

    auto compactedName = compactName(name);
    return _attributeIsDeclared(compactedName) ? compactedName : name;
}

QString LegacyNames::transform(const QString& transform)
{
    static const QRegularExpression actionPositionRe(uR"(^\s*(\[[^\]]*\])?\s*$)"_s);
    static const QRegularExpression attributePositionRe(uR"(\$((source|target)\.)?$)"_s);
    static const QRegularExpression parameterNamePositionRe(uR"(^\s*=(?!=))"_s);
    static const QRegularExpression parameterValuePositionRe(uR"(^\s*=\s*$)"_s);

    auto quotedStrings = quotedStringsIn(transform);
    std::vector<QString> newContents;
    newContents.reserve(quotedStrings.size());

    const GraphTransformFactory* factory = nullptr;
    GraphTransformParameter parameter;
    qsizetype parameterNameEnd = -1;

    for(const auto& quotedString : quotedStrings)
    {
        auto preceding = transform.left(quotedString._start);

        if(parameterNameEnd >= 0 && parameterValuePositionRe.match(
            transform.mid(parameterNameEnd, quotedString._start - parameterNameEnd)).hasMatch())
        {
            newContents.push_back(choiceValue(parameter, quotedString._content));
        }
        else if(&quotedString == &quotedStrings.front() && actionPositionRe.match(preceding).hasMatch())
        {
            newContents.push_back(transformName(quotedString._content));
            factory = _transformFactory(newContents.back());
        }
        else if(attributePositionRe.match(preceding).hasMatch())
            newContents.push_back(attributeName(quotedString._content));
        else if(parameterNamePositionRe.match(transform.mid(quotedString._end)).hasMatch())
        {
            newContents.push_back(parameterName(factory, quotedString._content));
            parameter = factory != nullptr ? factory->parameter(newContents.back()) : GraphTransformParameter();
            parameterNameEnd = quotedString._end;
            continue;
        }
        else
            newContents.push_back(quotedString._content);

        parameterNameEnd = -1;
    }

    auto renamedTransform = renamed(transform, quotedStrings, newContents);

    // Average Attribute named its attribute "Mean <A> of <B>", after the attributes it uses
    GraphTransformConfigParser legacyParser;
    GraphTransformConfigParser parser;

    if(legacyParser.parse(transform, "warnOnFailure"_no) && parser.parse(renamedTransform, "warnOnFailure"_no) &&
        parser.result()._action == u"AverageAttribute"_s)
    {
        const auto& legacyUsedAttributeNames = legacyParser.result().attributeNames();
        const auto& usedAttributeNames = parser.result().attributeNames();

        if(legacyUsedAttributeNames.size() == 2 && usedAttributeNames.size() == 2)
        {
            auto legacyMeanAttributeName = u"Mean %1 of %2"_s.arg(
                Attribute::prettify(legacyUsedAttributeNames.at(1)),
                Attribute::prettify(legacyUsedAttributeNames.at(0)));

            _attributeNames[legacyMeanAttributeName] =
                AverageAttributeTransform::meanAttributeName(
                usedAttributeNames.at(1),
                usedAttributeNames.at(0));
        }
    }

    return renamedTransform;
}

QString LegacyNames::visualisation(const QString& visualisation) const
{
    static const QRegularExpression attributePositionRe(uR"(^\s*(\[[^\]]*\])?\s*((source|target)\.)?$)"_s);

    auto quotedStrings = quotedStringsIn(visualisation);
    std::vector<QString> newContents;
    newContents.reserve(quotedStrings.size());

    for(const auto& quotedString : quotedStrings)
        newContents.push_back(quotedString._content);

    size_t channelIndex = 0;

    if(!quotedStrings.empty() &&
        attributePositionRe.match(visualisation.left(quotedStrings.front()._start)).hasMatch())
    {
        newContents.front() = attributeName(quotedStrings.front()._content);
        channelIndex = 1;
    }

    for(; channelIndex < quotedStrings.size(); channelIndex++)
    {
        const auto& quotedString = quotedStrings.at(channelIndex);

        if(quotedString._start > 0 && visualisation.at(quotedString._start - 1) == '.')
            continue;

        newContents.at(channelIndex) = channelName(quotedString._content);
        break;
    }

    return renamed(visualisation, quotedStrings, newContents);
}

void LegacyNames::renameAttributesIn(json& jsonObject, const std::vector<const char*>& keys) const
{
    if(!jsonObject.is_object())
        return;

    for(const auto* key : keys)
    {
        if(u::contains(jsonObject, key))
            renameAttributesInValue(jsonObject[key]);
    }
}

// Heuristically rewrite names from the old format to the new; this should cover
// all the cases we need to deal with from what we've already released
QString LegacyNames::compactName(QString name)
{
    if(std::ranges::all_of(name, [](QChar c) { return c.isLetterOrNumber(); }))
        return name;

    name.replace(u'%', u"Percent"_s);

    auto words = name.split(u' ', Qt::SkipEmptyParts);
    for(auto& word : words)
        word[0] = word.at(0).toUpper();

    auto compactedName = words.join(QString());
    compactedName.removeIf([](QChar c) { return !c.isLetterOrNumber(); });

    return compactedName;
}

QString LegacyNames::parameterName(const GraphTransformFactory* factory, const QString& name)
{
    auto newName = QString(name).remove(u' ');
    return factory != nullptr && !factory->parameter(newName).name().isEmpty() ? newName : name;
}

QString LegacyNames::choiceValue(const GraphTransformParameter& parameter, const QString& value)
{
    auto newValue = QString(value).remove(u' ');
    return parameter.initialValue().toStringList().contains(newValue) ? newValue : value;
}

QString LegacyNames::channelName(const QString& name)
{
    return QString(name).remove(u' ');
}

void LegacyNames::renameAttributesInValue(json& jsonValue) const
{
    if(jsonValue.is_string())
    {
        auto value = QString::fromStdString(jsonValue.get<std::string>());
        auto renamedValue = attributeName(value);

        // Table columns made from a parameterised attribute were named "Name › Parameter",
        // with a utf thin space either side of the ›
        static const QString parameterSeparator = u"\u2009\u203A\u2009"_s;
        auto separatorIndex = value.indexOf(parameterSeparator);
        if(renamedValue == value && separatorIndex > 0)
        {
            auto legacyName = value.left(separatorIndex);
            auto name = attributeName(legacyName);

            if(name != legacyName)
                renamedValue = u"%1.%2"_s.arg(name, value.mid(separatorIndex + parameterSeparator.size()));
        }

        if(renamedValue != value)
            jsonValue = renamedValue.toStdString();
    }
    else if(jsonValue.is_array())
    {
        for(auto& element : jsonValue)
            renameAttributesInValue(element);
    }
}

void renameEachIn(json& jsonArray, const std::function<QString(const QString&)>& renameFn)
{
    if(!jsonArray.is_array())
        return;

    for(auto& element : jsonArray)
    {
        if(element.is_string())
            element = renameFn(QString::fromStdString(element.get<std::string>())).toStdString();
    }
}
