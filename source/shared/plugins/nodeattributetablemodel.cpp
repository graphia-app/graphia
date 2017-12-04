#include "nodeattributetablemodel.h"

#include "usernodedata.h"

#include "shared/ui/iselectionmanager.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"
#include "shared/attributes/iattribute.h"
#include "shared/attributes/valuetype.h"

#include "shared/utils/container.h"

NodeAttributeTableModel::NodeAttributeTableModel() :
    QAbstractTableModel()
{}

void NodeAttributeTableModel::initialise(IDocument* document, UserNodeData* userNodeData)
{
    _document = document;
    _userNodeData = userNodeData;

    updateRoleNames();

    auto graphModel = _document->graphModel();

    auto modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributeAdded(const QString&)),
            this, SLOT(onAttributeAdded(const QString&)), Qt::DirectConnection);
    connect(modelQObject, SIGNAL(attributeRemoved(const QString&)),
            this, SLOT(onAttributeRemoved(const QString&)), Qt::DirectConnection);

    auto graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
    connect(graphQObject, SIGNAL(graphChanged(const Graph*, bool)),
            this, SLOT(onGraphChanged(const Graph*, bool)), Qt::DirectConnection);
}

QStringList NodeAttributeTableModel::columnNames() const
{
    QStringList list;
    list.reserve(_userNodeData->numUserDataVectors());

    for(const auto& userDataVector : *_userNodeData)
        list.append(userDataVector.name());

    for(auto& attributeName : _document->graphModel()->attributeNames(ElementType::Node))
    {
        if(!u::contains(list, attributeName))
            list.append(attributeName);
    }

    return list;
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::mutex> lock;
    //FIXME depending on the precise reasons for calling update(), it isn't
    // necessarily required to regenerate all the data from scratch. e.g.
    // if it's in response to a selection change, only the NodeSelectedRole
    // actually needs to change, and probably begin/endResetModel can be
    // avoided too

    Table updatedData;

    for(int row = 0; row < rowCount(); row++)
    {
        updatedData.emplace_back(_roleNames.size());
        auto& dataRow = updatedData.back();

        NodeId nodeId = _userNodeData->nodeIdForRowIndex(row);

        if(!_document->graphModel()->graph().containsNodeId(nodeId))
        {
            // The graph doesn't necessarily have a node for every row since
            // it may have been transformed, leaving empty rows
            continue;
        }

        for(int roleNum = 0; roleNum < _roleNames.size(); roleNum++)
        {
            int role = Qt::UserRole + 1 + roleNum;

            if(role == Roles::NodeIdRole)
                dataRow[roleNum] = static_cast<int>(nodeId);
            else if(role == Roles::NodeSelectedRole)
                dataRow[roleNum] = _document->selectionManager()->nodeIsSelected(nodeId);
            else
            {
                auto* attribute = _document->graphModel()->attributeByName(_roleNames[role]);
                if(attribute != nullptr)
                    dataRow[roleNum] = attribute->valueOf(nodeId);
            }
        }
    }

    _updatedData.emplace_back(std::move(updatedData));

    // Notify the main thread that the data has changed
    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateComplete()
{
    std::unique_lock<std::mutex> lock;

    beginResetModel();
    _cachedData = std::move(_updatedData.front());
    _updatedData.pop_front();
    endResetModel();
}

void NodeAttributeTableModel::onGraphChanged(const Graph*, bool changeOccurred)
{
    if(changeOccurred)
        update();
}

void NodeAttributeTableModel::updateRoleNames()
{
    // Regenerate rolenames
    _roleNames.clear();
    _roleNames.insert(Roles::NodeIdRole, "nodeId");
    _roleNames.insert(Roles::NodeSelectedRole, "nodeSelected");

    int nextRole = Roles::FirstAttributeRole;
    const auto names = columnNames();
    for(const auto& name : names)
    {
        _roleNames.insert(nextRole, name.toUtf8());
        nextRole++;
    }

    _columnCount = names.size();

    emit columnNamesChanged();
}

bool NodeAttributeTableModel::columnIsCalculated(const QString& columnName) const
{
    return !u::contains(_userNodeData->vectorNames(), columnName);
}

void NodeAttributeTableModel::moveFocusToNodeForRowIndex(size_t row)
{
    auto nodeId = _userNodeData->nodeIdForRowIndex(row);
    _document->moveFocusToNode(nodeId);
}

bool NodeAttributeTableModel::columnIsFloatingPoint(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr)
        return attribute->valueType() == ValueType::Float;

    return false;
}

void NodeAttributeTableModel::onAttributeAdded(const QString& name)
{
    // Recreate rolenames in the model if the attribute is new
    if(!u::contains(_roleNames.values(), name.toUtf8()))
    {
        updateRoleNames();
        update();
    }
}

void NodeAttributeTableModel::onAttributeRemoved(const QString& name)
{
    if(u::contains(_roleNames.values(), name.toUtf8()))
    {
        updateRoleNames();
        update();
    }
}

int NodeAttributeTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numValues());
}

int NodeAttributeTableModel::columnCount(const QModelIndex&) const
{
    return _columnCount;
}

QVariant NodeAttributeTableModel::data(const QModelIndex& index, int role) const
{
    size_t row = index.row();
    if(row >= _cachedData.size())
        return {};

    const auto& dataRow = _cachedData.at(row);

    size_t column = (role - Qt::UserRole - 1);
    if(column >= dataRow.size())
        return {};

    auto cachedValue = dataRow.at(column);
    return cachedValue;
}

void NodeAttributeTableModel::onSelectionChanged()
{
    update();
}
