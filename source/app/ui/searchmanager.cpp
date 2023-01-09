/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "searchmanager.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"
#include "attributes/conditionfncreator.h"

#include "shared/utils/container.h"

#include <QRegularExpression>

#include <algorithm>

SearchManager::SearchManager(const GraphModel& graphModel) :
    _graphModel(&graphModel)
{}

void SearchManager::findNodes(QString term, Flags<FindOptions> options,
    QStringList attributeNames, FindSelectStyle selectStyle)
{
    _term = term;
    _options = options;
    _attributeNames = std::move(attributeNames);
    _selectStyle = selectStyle;

    if(term.isEmpty())
    {
        clearFoundNodeIds();
        return;
    }

    NodeIdSet foundNodeIds;

    // If no attributes are specified, search them all
    if(_attributeNames.empty())
    {
        for(auto& attributeName : _graphModel->attributeNames(ElementType::Node))
            _attributeNames.append(attributeName);
    }

    std::vector<Attribute> attributes;
    for(auto& attributeName : _attributeNames)
    {
        if(!_graphModel->attributeExists(attributeName))
            continue;

        auto attribute = _graphModel->attributeValueByName(attributeName);

        if(attribute.testFlag(AttributeFlag::Searchable) &&
            attribute.elementType() == ElementType::Node)
        {
            attributes.emplace_back(attribute);
        }
    }

    // None of the given attributes are searchable
    if(!_attributeNames.empty() && attributes.empty())
    {
        clearFoundNodeIds();
        return;
    }

    QRegularExpression::PatternOptions reOptions;

    if(options.test(FindOptions::MatchExact))
    {
        term = QRegularExpression::escape(term);
        term = QStringLiteral("^%1$").arg(term);
    }
    else
    {
        if(!options.test(FindOptions::MatchUsingRegex))
            term = QRegularExpression::escape(term);

        if(options.test(FindOptions::MatchWholeWords))
            term = QStringLiteral(R"(\b(%1)\b)").arg(term);

        if(!options.test(FindOptions::MatchCase))
            reOptions.setFlag(QRegularExpression::CaseInsensitiveOption);
    }

    const QRegularExpression re(term, reOptions);

    if(re.isValid())
    {
        ConditionFnOp::String op = ConditionFnOp::String::MatchesRegex;
        if(reOptions.testFlag(QRegularExpression::CaseInsensitiveOption))
            op = ConditionFnOp::String::MatchesRegexCaseInsensitive;

        std::vector<NodeConditionFn> conditionFns;
        for(auto& attribute : attributes)
        {
            auto conditionFn = CreateConditionFnFor::node(attribute, op, term);

            if(conditionFn != nullptr)
                conditionFns.emplace_back(conditionFn);
        }

        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            // We can't add tail nodes to the results since merge sets can only be found
            // using head nodes... (cont.)
            if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                continue;

            const auto& mergedNodeIds = _graphModel->graph().mergedNodeIdsForNodeId(nodeId);
            bool match = false;

            // Fall back on a node name search if there are no attributes provided
            if(attributes.empty())
                match = re.match(_graphModel->nodeNames().at(nodeId)).hasMatch();

            if(!match)
            {
                match = std::any_of(conditionFns.begin(), conditionFns.end(),
                [&mergedNodeIds](const auto& conditionFn)
                {
                    // ...but we still match against the tails... (cont.)
                    return std::any_of(mergedNodeIds.begin(), mergedNodeIds.end(),
                    [&conditionFn](auto mergedNodeId)
                    {
                       return conditionFn(mergedNodeId);
                    });
                });
            }

            if(match)
            {
                // ...so that the entire merge set of the head is found, even if
                // it's only a subset of the merge set that actually matched
                foundNodeIds.insert(mergedNodeIds.begin(), mergedNodeIds.end());
            }
        }
    }

    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    const bool changed = u::setsDiffer(_foundNodeIds, foundNodeIds);

    _foundNodeIds = std::move(foundNodeIds);

    if(changed)
        emit foundNodeIdsChanged(this);
}

void SearchManager::clearFoundNodeIds()
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    const bool changed = !_foundNodeIds.empty();
    _foundNodeIds.clear();

    if(changed)
        emit foundNodeIdsChanged(this);
}

void SearchManager::refresh()
{
    findNodes(_term, _options, _attributeNames, _selectStyle);
}

NodeIdSet SearchManager::foundNodeIds() const
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    return _foundNodeIds;
}
