#include "searchmanager.h"

#include "../graph/graphmodel.h"

#include "shared/utils/utils.h"

#include <QRegularExpression>

SearchManager::SearchManager(const GraphModel& graphModel) :
    _graphModel(&graphModel)
{}

void SearchManager::findNodes(const QString& regex, std::vector<QString> dataFieldNames)
{
    _regex = regex;
    _dataFieldNames = dataFieldNames;

    if(_regex.isEmpty())
    {
        clearFoundNodeIds();
        return;
    }

    NodeIdSet foundNodeIds;

    // If no data fields are specified, search them all
    if(_dataFieldNames.empty())
    {
        for(auto& dataFieldName : _graphModel->dataFieldNames(DataFieldElementType::Node))
            _dataFieldNames.emplace_back(dataFieldName);
    }

    std::vector<const DataField*> dataFields;
    for(auto& dataFieldName : _dataFieldNames)
    {
        const auto* dataField = &_graphModel->dataFieldByName(dataFieldName);

        if(dataField->searchable() && dataField->elementType() == DataFieldElementType::Node)
            dataFields.emplace_back(dataField);
    }

    QRegularExpression re(_regex, QRegularExpression::CaseInsensitiveOption);

    if(re.isValid())
    {
        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            bool match = re.match(_graphModel->nodeNames().at(nodeId)).hasMatch();

            if(!match)
            {
                for(auto& dataField : dataFields)
                {
                    auto conditionFn = dataField->createNodeConditionFn(ConditionFnOp::MatchesRegex, re);
                    match = conditionFn(nodeId);
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
    _foundNodeIds.clear();
    emit foundNodeIdsChanged(this);
}

void SearchManager::refresh()
{
    findNodes(_regex, _dataFieldNames);
}

bool SearchManager::SearchManager::nodeWasFound(NodeId nodeId) const
{
    return u::contains(_foundNodeIds, nodeId);
}
