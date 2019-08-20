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
    _roleNames.insert(Roles::NodeIdRole, "nodeId");
    _roleNames.insert(Roles::NodeSelectedRole, "nodeSelected");
    _roleNames.insert(Qt::DisplayRole, "display");

    _document = document;
    _userNodeData = userNodeData;
    _graph = &_document->graphModel()->graph();

    auto graphModel = _document->graphModel();

    updateColumnNames();

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
            _document->reportProblem(QStringLiteral("Attribute not found: %1").arg(attributeName));
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

int NodeAttributeTableModel::columnIndexForAttributeValue(QString attributeValue)
{
    return columnNames().indexOf(attributeValue);
}

QVariant NodeAttributeTableModel::dataValue(int row, const IAttribute* attribute) const
{
    if(attribute != nullptr && attribute->isValid())
    {
        auto nodeId = _userNodeData->elementIdForIndex(row);

        if(!attribute->valueMissingOf(nodeId))
            return attribute->valueOf(nodeId);
    }

    return {};
}

void NodeAttributeTableModel::onColumnAdded(int columnIndex)
{
    if(columnIndex < _pendingData.size())
        _pendingData.insert(_pendingData.begin() + columnIndex, {{}});
    else
        _pendingData.resize(columnIndex + 1);

    //updateColumn(role, _pendingData.at(index));
}

void NodeAttributeTableModel::onColumnRemoved(int columnIndex)
{
    Q_ASSERT(columnIndex < _pendingData.size());
    _pendingData.erase(_pendingData.begin() + columnIndex);
}

void NodeAttributeTableModel::updateColumnNames()
{
    _columnNames = columnNames();
}

void NodeAttributeTableModel::updateRole(const QString& attributeName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    size_t index = columnIndexForAttributeValue(attributeName);

    Q_ASSERT(index < _pendingData.size());
    auto& column = _pendingData.at(index);

    updateColumn(Qt::DisplayRole, attributeName, column);

    QMetaObject::invokeMethod(this, "onUpdateRoleComplete", Q_ARG(QString, attributeName));
}

void NodeAttributeTableModel::updateColumn(int role, const QString& attributeName, NodeAttributeTableModel::Column& column)
{
    column.resize(rowCount());

    const auto* attribute = _document->graphModel()->attributeByName(attributeName);
    for(int row = 0; row < rowCount(); row++)
    {
        NodeId nodeId = _userNodeData->elementIdForIndex(row);

        //if(row == 0)
            //column[row] = attributeName;

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
            column[row] = dataValue(row, attribute);
    }
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    _pendingData.clear();

    updateColumn(Roles::NodeSelectedRole, "", _nodeSelectedColumn);
    updateColumn(Roles::NodeIdRole, "", _nodeIdColumn);

    int i = 0;
    for(auto columnName : columnNames())
    {
        _pendingData.emplace_back(i++);
        updateColumn(Qt::DisplayRole, columnName, _pendingData.back());
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateRoleComplete(const QString& attributeName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    emit layoutAboutToBeChanged();
    int column = columnIndexForAttributeValue(attributeName);
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
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    beginResetModel();
    _data = _pendingData;
    endResetModel();
    emit columnNamesChanged();
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

bool NodeAttributeTableModel::rowVisible(size_t row) const
{
    return _nodeSelectedColumn[row].toBool();
}

QString NodeAttributeTableModel::columnHeaders(size_t column) const
{
    return columnNames().at(column);
}

void NodeAttributeTableModel::onAttributesChanged(const QStringList& added, const QStringList& removed)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    auto byteArrayRoleNames = _roleNames.values();
    QStringList currentRoleNames;
    currentRoleNames.reserve(_roleNames.size());
    for(const auto& roleName : qAsConst(byteArrayRoleNames))
        currentRoleNames.append(roleName);

    QSet<QString> currentRoleNamesSet(currentRoleNames.begin(), currentRoleNames.end());
    QSet<QString> addedSet(added.begin(), added.end());
    QSet<QString> removedSet(removed.begin(), removed.end());

    Q_ASSERT(added.isEmpty() || currentRoleNamesSet.intersect(addedSet).isEmpty());

    // Ignore attribute names we aren't using (they may not be node attributes)
    auto filteredRemoved = currentRoleNamesSet.intersect(removedSet).values();

    if(added.isEmpty() && filteredRemoved.isEmpty())
    {
        // There is no structural change to the table, but some roles' values
        // may have changed, so we need to update these individually
        while(!_rolesRequiringUpdates.empty())
        {
            updateRole(_roleNames.value(_rolesRequiringUpdates.back()));
            _rolesRequiringUpdates.pop_back();
        }

        return;
    }

    // Any roles requiring an update will be taken care of en-masse, in onUpdateComplete
    _rolesRequiringUpdates.clear();

    for(const auto& name : filteredRemoved)
    {
        auto columnIndex = _columnNames.indexOf(name);
        onColumnRemoved(columnIndex);
        emit columnRemoved(columnIndex, name);
    }

    for(const auto& name : added)
    {
        auto attribute = _document->graphModel()->attributeByName(name);

        // We only care about node attributes, obviously
        if(attribute->elementType() != ElementType::Node || attribute->hasParameter())
            continue;

        auto columnIndex = _columnNames.indexOf(name);

        onColumnAdded(columnIndex);
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
    return _userNodeData->numUserDataVectors();
}

QVariant NodeAttributeTableModel::data(const QModelIndex& index, int role) const
{
    size_t column = index.column();
    if(role == Qt::DisplayRole)
    {
        if(column >= _data.size())
            return {};

        const auto& dataColumn = _data.at(column);

        size_t row = static_cast<size_t>(index.row());
        if(row >= dataColumn.size())
            return {};

        auto cachedValue = dataColumn.at(row);
        return cachedValue;
    }
    else if (role == Roles::NodeSelectedRole){
        size_t row = static_cast<size_t>(index.row());
        return _nodeSelectedColumn[row];
    }
    return {};
}

void NodeAttributeTableModel::onSelectionChanged()
{
    updateColumn(Roles::NodeSelectedRole, "", _nodeSelectedColumn);
    emit selectionChanged();
}
