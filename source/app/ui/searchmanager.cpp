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
        auto attribute = _graphModel->attributeValueByName(attributeName);

        if(attribute.searchable() && attribute.elementType() == ElementType::Node)
            attributes.emplace_back(attribute);
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

    QRegularExpression re(term, reOptions);

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
            // From a search results point of view, we only care about head nodes...
            if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                continue;

            bool match = false;

            // Fall back on a node name search if there are no attributes provided
            if(attributes.empty())
                match = re.match(_graphModel->nodeNames().at(nodeId)).hasMatch();

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
    findNodes(_term, _options, _attributeNames, _selectStyle);
}

bool SearchManager::SearchManager::nodeWasFound(NodeId nodeId) const
{
    return u::contains(_foundNodeIds, nodeId);
}
