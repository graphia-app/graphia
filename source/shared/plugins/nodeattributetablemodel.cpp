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
    _graph = &_document->graphModel()->graph();

    updateRoleNames();

    auto graphModel = _document->graphModel();

    auto modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributeAdded(const QString&)),
            this, SLOT(onAttributeAdded(const QString&)), Qt::DirectConnection);
    connect(modelQObject, SIGNAL(attributeRemoved(const QString&)),
            this, SLOT(onAttributeRemoved(const QString&)), Qt::DirectConnection);
    connect(modelQObject, SIGNAL(attributeValuesChanged(const QString&)),
            this, SLOT(onAttributeValuesChanged(const QString&)), Qt::DirectConnection);

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

void NodeAttributeTableModel::addRole(int role)
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    size_t index = role - (Qt::UserRole + 1);

    if(index < _pendingData.size())
        _pendingData.insert(_pendingData.begin() + index, {});
    else
        _pendingData.resize(index + 1);

    updateColumn(role, _pendingData.at(index));

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::removeRole(int role)
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    size_t index = role - (Qt::UserRole + 1);

    Q_ASSERT(index < _pendingData.size());
    _pendingData.erase(_pendingData.begin() + index);

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::updateRole(int role)
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    size_t index = role - (Qt::UserRole + 1);

    Q_ASSERT(index < _pendingData.size());
    auto& column = _pendingData.at(index);

    updateColumn(role, column);

    QMetaObject::invokeMethod(this, "onUpdateRoleComplete", Q_ARG(int, role));
}

void NodeAttributeTableModel::updateColumn(int role, NodeAttributeTableModel::Column& column)
{
    column.resize(rowCount());

    for(int row = 0; row < rowCount(); row++)
    {
        NodeId nodeId = _userNodeData->elementIdForRowIndex(row);

        if(!_document->graphModel()->graph().containsNodeId(nodeId))
        {
            // The graph doesn't necessarily have a node for every row since
            // it may have been transformed, leaving empty rows
            column[row] = {};
        }
        else if(role == Roles::NodeIdRole)
            column[row] = static_cast<int>(nodeId);
        else if(role == Roles::NodeSelectedRole)
            column[row] = _document->selectionManager()->nodeIsSelected(nodeId);
        else
            column[row] = dataValue(row, role);
    }
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    _pendingData.clear();

    for(int column = 0; column < _roleNames.size(); column++)
    {
        int role = Qt::UserRole + 1 + column;

        _pendingData.emplace_back(rowCount());
        updateColumn(role, _pendingData.back());
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateRoleComplete(int role)
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    emit layoutAboutToBeChanged();
    int column = role - (Qt::UserRole + 1);
    _data.at(column) = _pendingData.at(column);

    //FIXME: Emitting dataChanged /should/ be faster than doing a layoutChanged, but
    // for some reason it's not, even with https://codereview.qt-project.org/#/c/219278/
    // applied. Most of the performance seems to be lost deep in TableView's JS so perhaps
    // we should just ditch TableView and implement our own custom table, that we can
    // control better. Certainly the internet suggests using ListView:
    //      https://stackoverflow.com/a/43856015
    //      https://stackoverflow.com/a/45188582
    // Also, note that NodeAttributeTableView currently relies on layoutChanged, so if
    // what we emit changes, we need to be account for it there too.
    //emit dataChanged(index(0, column), index(rowCount() - 1, column), {role});

    emit layoutChanged();
}

void NodeAttributeTableModel::onUpdateComplete()
{
    std::unique_lock<std::mutex> lock(_updateMutex);

    beginResetModel();
    _data = _pendingData;
    endResetModel();
}

void NodeAttributeTableModel::onGraphChanged(const Graph*, bool changeOccurred)
{
    if(changeOccurred)
    {
        _graph->setPhase(tr("Attribute Table"));
        update();
        _graph->clearPhase();
    }
}

void NodeAttributeTableModel::updateRoleNames()
{
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
        addRole(_roleNames.key(name.toUtf8()));

        auto index = _columnNames.indexOf(name);
        if(index >= 0)
            emit columnAdded(index, name);
    }
}

void NodeAttributeTableModel::onAttributeRemoved(const QString& name)
{
    if(u::contains(_roleNames.values(), name.toUtf8()))
    {
        auto index = _columnNames.indexOf(name);

        int role = _roleNames.key(name.toUtf8());
        updateRoleNames();
        removeRole(role);

        if(index >= 0)
            emit columnRemoved(index, name);
    }
}

void NodeAttributeTableModel::onAttributeValuesChanged(const QString& name)
{
    if(u::contains(_roleNames.values(), name.toUtf8()))
    {
        int role = _roleNames.key(name.toUtf8());
        updateRole(role);
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
    size_t column = (role - Qt::UserRole - 1);
    if(column >= _data.size())
        return {};

    const auto& dataColumn = _data.at(column);

    size_t row = index.row();
    if(row >= dataColumn.size())
        return {};

    auto cachedValue = dataColumn.at(row);
    return cachedValue;
}

void NodeAttributeTableModel::onSelectionChanged()
{
    updateRole(Roles::NodeSelectedRole);
}
