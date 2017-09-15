#include "searchmanager.h"

#include "graph/graphmodel.h"
#include "attributes/conditionfncreator.h"

#include "shared/utils/utils.h"

#include <QRegularExpression>

#include <algorithm>

SearchManager::SearchManager(const GraphModel& graphModel) :
    _graphModel(&graphModel)
{}

void SearchManager::findNodes(const QString& regex, std::vector<QString> attributeNames)
{
    _regex = regex;
    _attributeNames = std::move(attributeNames);

    if(_regex.isEmpty())
    {
        clearFoundNodeIds();
        return;
    }

    NodeIdSet foundNodeIds;

    // If no attributes are specified, search them all
    if(_attributeNames.empty())
    {
        for(auto& attributeName : _graphModel->attributeNames(ElementType::Node))
            _attributeNames.emplace_back(attributeName);
    }

    std::vector<Attribute> attributes;
    for(auto& attributeName : _attributeNames)
    {
        auto attribute = _graphModel->attributeValueByName(attributeName);

        if(attribute.searchable() && attribute.elementType() == ElementType::Node)
            attributes.emplace_back(attribute);
    }

    QRegularExpression re(_regex, QRegularExpression::CaseInsensitiveOption);

    if(re.isValid())
    {
        std::vector<NodeConditionFn> conditionFns;
        for(auto& attribute : attributes)
        {
            auto conditionFn = CreateConditionFnFor::node(attribute,
                ConditionFnOp::String::MatchesRegexCaseInsensitive, _regex);

            if(conditionFn != nullptr)
                conditionFns.emplace_back(conditionFn);
        }

        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            // From a search results point of view, we only care about head nodes...
            if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                continue;

            bool match = re.match(_graphModel->nodeNames().at(nodeId)).hasMatch();

            if(!match)
            {
                for(const auto& conditionFn : conditionFns)
                {
                    // ...but we still match against the tails
                    const auto& mergedNodeIds = _graphModel->graph().mergedNodeIdsForNodeId(nodeId);
                    match = std::any_of(mergedNodeIds.begin(), mergedNodeIds.end(),
                    [&conditionFn](auto mergedNodeId)
                    {
                       return conditionFn(mergedNodeId);
                    });

                    if(match)
                        break;
                }
            }

            if(match)
                foundNodeIds.emplace(nodeId);
        }
    }

    bool changed = u::setsDiffer(_foundNodeIds, foundNodeIds);

    _foundNodeIds = std::move(foundNodeIds);

    if(changed)
        emit foundNodeIdsChanged(this);
}

void SearchManager::clearFoundNodeIds()
{
    bool changed = !_foundNodeIds.empty();
    _foundNodeIds.clear();

    if(changed)
        emit foundNodeIdsChanged(this);
}

void SearchManager::refresh()
{
    findNodes(_regex, _attributeNames);
}

bool SearchManager::SearchManager::nodeWasFound(NodeId nodeId) const
{
    return u::contains(_foundNodeIds, nodeId);
}
