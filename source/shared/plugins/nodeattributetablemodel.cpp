#include "nodeattributetablemodel.h"

#include "userelementdata.h"

#include "../crashhandler.h"

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
    connect(modelQObject, SIGNAL(attributesChanged(const QStringList&, const QStringList&)),
            this, SLOT(onAttributesChanged(const QStringList&, const QStringList&)), Qt::DirectConnection);
    connect(modelQObject, SIGNAL(attributeValuesChanged(const QStringList&)),
            this, SLOT(onAttributeValuesChanged(const QStringList&)), Qt::DirectConnection);

    auto graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
    connect(graphQObject, SIGNAL(graphChanged(const Graph*, bool)),
            this, SLOT(onGraphChanged(const Graph*, bool)), Qt::DirectConnection);
}

QStringList NodeAttributeTableModel::columnNames() const
{
    QStringList list;
    list.reserve(_userNodeData->numUserDataVectors());

    for(const auto& [name, userDataVector] : *_userNodeData)
        list.append(userDataVector.name());

    for(auto& attributeName : _document->graphModel()->attributeNames(ElementType::Node))
    {
        auto attribute = _document->graphModel()->attributeByName(attributeName);

        // If this happens, there is probably something weird about the name
        Q_ASSERT(attribute != nullptr);
        if(attribute == nullptr)
        {
            S(CrashHandler)->submitMinidump(QStringLiteral("Attribute not found: %1").arg(attributeName));
            continue;
        }

        // We can't show parameterised attributes in the table
        if(attribute->hasParameter())
            continue;

        if(!u::contains(list, attributeName))
            list.append(attributeName);
    }

    return list;
}

QVariant NodeAttributeTableModel::dataValue(int row, int role) const
{
    auto* attribute = _document->graphModel()->attributeByName(_roleNames[role]);
    if(attribute != nullptr && attribute->isValid())
    {
        auto nodeId = _userNodeData->elementIdForIndex(row);

        if(!attribute->valueMissingOf(nodeId))
            return attribute->valueOf(nodeId);
    }

    return {};
}

void NodeAttributeTableModel::onRoleAdded(int role)
{
    size_t index = role - (Qt::UserRole + 1);

    if(index < _pendingData.size())
        _pendingData.insert(_pendingData.begin() + index, {{}});
    else
        _pendingData.resize(index + 1);

    updateColumn(role, _pendingData.at(index));
}

void NodeAttributeTableModel::onRoleRemoved(int role)
{
    size_t index = role - (Qt::UserRole + 1);

    Q_ASSERT(index < _pendingData.size());
    _pendingData.erase(_pendingData.begin() + index);
}

void NodeAttributeTableModel::updateRole(int role)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

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
        NodeId nodeId = _userNodeData->elementIdForIndex(row);

        if(nodeId.isNull() || !_document->graphModel()->graph().containsNodeId(nodeId))
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
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

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
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    size_t index = role - (Qt::UserRole + 1);

    Q_ASSERT(index < _data.size() && index < _pendingData.size());
    if(index >= _data.size() || index >= _pendingData.size())
        return;

    emit layoutAboutToBeChanged();
    _data.at(index) = _pendingData.at(index);

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
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

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
    auto nodeId = _userNodeData->elementIdForIndex(row);
    _document->moveFocusToNode(nodeId);
}

bool NodeAttributeTableModel::columnIsFloatingPoint(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr && attribute->isValid())
        return attribute->valueType() == ValueType::Float;

    return false;
}

bool NodeAttributeTableModel::columnIsNumerical(const QString& columnName) const
{
    const auto* graphModel = _document->graphModel();
    const auto* attribute = graphModel->attributeByName(columnName);

    if(attribute != nullptr && attribute->isValid())
        return attribute->valueType() & ValueType::Numerical;

    return false;
}

void NodeAttributeTableModel::onAttributesChanged(const QStringList& added, const QStringList& removed)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    auto byteArrayRoleNames =_roleNames.values();
    QStringList currentRoleNames;
    currentRoleNames.reserve(_roleNames.size());
    for(const auto& roleName : qAsConst(byteArrayRoleNames))
        currentRoleNames.append(roleName);

    Q_ASSERT(added.isEmpty() || currentRoleNames.toSet().intersect(added.toSet()).isEmpty());

    // Ignore attribute names we aren't using (they may not be node attributes)
    auto filteredRemoved = currentRoleNames.toSet().intersect(removed.toSet()).toList();

    if(added.isEmpty() && filteredRemoved.isEmpty())
    {
        // There is no structural change to the table, but some roles' values
        // may have changed, so we need to update these individually
        while(!_rolesRequiringUpdates.empty())
        {
            updateRole(_rolesRequiringUpdates.back());
            _rolesRequiringUpdates.pop_back();
        }

        return;
    }

    // Any roles requiring an update will be taken care of en-masse, in onUpdateComplete
    _rolesRequiringUpdates.clear();

    for(const auto& name : filteredRemoved)
    {
        auto columnIndex = _columnNames.indexOf(name);
        int role = _roleNames.key(name.toUtf8());

        onRoleRemoved(role);
        emit columnRemoved(columnIndex, name);
    }

    updateRoleNames();

    for(const auto& name : added)
    {
        auto attribute = _document->graphModel()->attributeByName(name);

        // We only care about node attributes, obviously
        if(attribute->elementType() != ElementType::Node || attribute->hasParameter())
            continue;

        auto columnIndex = _columnNames.indexOf(name);
        int role = _roleNames.key(name.toUtf8());

        onRoleAdded(role);
        emit columnAdded(columnIndex, name);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onAttributeValuesChanged(const QStringList& attributeNames)
{
    for(const auto& attributeName : attributeNames)
    {
        if(!u::contains(_roleNames.values(), attributeName.toUtf8()))
            continue;

        int role = _roleNames.key(attributeName.toUtf8());
        _rolesRequiringUpdates.push_back(role);

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
    size_t column = role - (Qt::UserRole + 1);
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
