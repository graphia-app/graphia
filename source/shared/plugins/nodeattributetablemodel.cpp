#include "nodeattributetablemodel.h"

#include "userelementdata.h"

#include "shared/ui/iselectionmanager.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"
#include "shared/attributes/iattribute.h"
#include "shared/attributes/valuetype.h"

#include "shared/utils/container.h"

#include <QtGlobal>

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

QVariant NodeAttributeTableModel::dataValue(int row, int role) const
{
    auto* attribute = _document->graphModel()->attributeByName(_roleNames[role]);
    if(attribute != nullptr)
    {
        auto nodeId = _userNodeData->elementIdForRowIndex(row);

        if(!attribute->valueMissingOf(nodeId))
            return attribute->valueOf(nodeId);
    }

    return {};
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::mutex> lock;
    //FIXME depending on the precise reasons for calling update(), it isn't
    // necessarily required to regenerate all the data from scratch. e.g.
    // if it's in response to a selection change, only the NodeSelectedRole
    // actually needs to change, and probably begin/endResetModel can be
    // avoided too

    Table updatedData(rowCount() * _roleNames.size());

    for(int row = 0; row < rowCount(); row++)
    {
        NodeId nodeId = _userNodeData->elementIdForRowIndex(row);

        if(!_document->graphModel()->graph().containsNodeId(nodeId))
        {
            // The graph doesn't necessarily have a node for every row since
            // it may have been transformed, leaving empty rows
            continue;
        }

        for(int column = 0; column < _roleNames.size(); column++)
        {
            int role = Qt::UserRole + 1 + column;
            int index = column + (row * _roleNames.size());

            if(role == Roles::NodeIdRole)
                updatedData[index] = static_cast<int>(nodeId);
            else if(role == Roles::NodeSelectedRole)
                updatedData[index] = _document->selectionManager()->nodeIsSelected(nodeId);
            else
                updatedData[index] = dataValue(row, role);
        }
    }

    _updatedDatas.emplace_back(std::move(updatedData));

    // Notify the main thread that the data has changed
    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateComplete()
{
    std::unique_lock<std::mutex> lock;

    beginResetModel();
    _cachedData = std::move(_updatedDatas.front());
    _updatedDatas.pop_front();
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
    _columnNames = columnNames();
    for(const auto& name : qAsConst(_columnNames))
    {
        _roleNames.insert(nextRole, name.toUtf8());
        nextRole++;
    }

    _columnCount = _columnNames.size();

    emit columnNamesChanged();
}

bool NodeAttributeTableModel::columnIsCalculated(const QString& columnName) const
{
    return !u::contains(_userNodeData->vectorNames(), columnName);
}

bool NodeAttributeTableModel::columnIsHiddenByDefault(const QString&) const
{
    return false;
}

void NodeAttributeTableModel::moveFocusToNodeForRowIndex(size_t row)
{
    auto nodeId = _userNodeData->elementIdForRowIndex(row);
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
    size_t column = (role - Qt::UserRole - 1);

    size_t dataIndex = column + (row * static_cast<size_t>(_roleNames.size()));

    if(dataIndex > _cachedData.size())
        return {};

    auto cachedValue = _cachedData.at(dataIndex);
    return cachedValue;
}

void NodeAttributeTableModel::onSelectionChanged()
{
    update();
}
