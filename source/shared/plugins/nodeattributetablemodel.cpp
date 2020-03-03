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

    const auto* graphModel = _document->graphModel();

    updateColumnNames();

    const auto* modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributesChanged(const QStringList&, const QStringList&)),
            this, SLOT(onAttributesChanged(const QStringList&, const QStringList&)), Qt::DirectConnection);
    connect(modelQObject, SIGNAL(attributeValuesChanged(const QStringList&)),
            this, SLOT(onAttributeValuesChanged(const QStringList&)), Qt::DirectConnection);

    const auto* graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
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
        const auto* attribute = _document->graphModel()->attributeByName(attributeName);
        Q_ASSERT(attribute != nullptr);

        // We can't show parameterised attributes in the table
        if(attribute->hasParameter())
            continue;

        if(!u::contains(list, attributeName))
            list.append(attributeName);
    }

    return list;
}

int NodeAttributeTableModel::indexForColumnName(const QString& columnName)
{
    auto index = _columnNames.indexOf(columnName);
    Q_ASSERT(index >= 0);
    return index;
}

QVariant NodeAttributeTableModel::dataValue(size_t row, const QString& columnName) const
{
    const auto* attribute = _document->graphModel()->attributeByName(columnName);
    if(attribute != nullptr && attribute->isValid())
    {
        auto nodeId = _userNodeData->elementIdForIndex(row);

        if(!attribute->valueMissingOf(nodeId))
            return attribute->valueOf(nodeId);
    }

    return {};
}

void NodeAttributeTableModel::onColumnAdded(size_t columnIndex)
{
    auto columnName = _columnNames.at(static_cast<int>(columnIndex));

    if(columnIndex < _pendingData.size())
        _pendingData.insert(_pendingData.begin() + static_cast<int>(columnIndex), {{}});
    else
        _pendingData.resize(columnIndex + 1);

    auto& column = _pendingData.at(columnIndex);
    updateColumn(Qt::DisplayRole, column, columnName);
}

void NodeAttributeTableModel::onColumnRemoved(size_t columnIndex)
{
    Q_ASSERT(columnIndex < _pendingData.size());
    _pendingData.erase(_pendingData.begin() + static_cast<int>(columnIndex));
}

void NodeAttributeTableModel::updateColumnNames()
{
    _columnNames = columnNames();
    _columnCount = _columnNames.size();
    emit columnNamesChanged();
}

void NodeAttributeTableModel::updateAttribute(const QString& attributeName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    auto index = static_cast<size_t>(indexForColumnName(attributeName));

    Q_ASSERT(index < _pendingData.size());
    auto& column = _pendingData.at(index);

    updateColumn(Qt::DisplayRole, column, attributeName);

    QMetaObject::invokeMethod(this, "onUpdateColumnComplete", Q_ARG(QString, attributeName));
}

void NodeAttributeTableModel::updateColumn(int role, NodeAttributeTableModel::Column& column,
    const QString& columnName)
{
    column.resize(static_cast<size_t>(rowCount()));

    for(size_t row = 0; row < static_cast<size_t>(rowCount()); row++)
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
            column[row] = dataValue(row, columnName);
    }
}

void NodeAttributeTableModel::update()
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    _pendingData.clear();

    updateColumn(Roles::NodeSelectedRole, _nodeSelectedColumn);
    updateColumn(Roles::NodeIdRole, _nodeIdColumn);

    for(const auto& columnName : _columnNames)
    {
        _pendingData.emplace_back(rowCount());
        updateColumn(Qt::DisplayRole, _pendingData.back(), columnName);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateColumnComplete(const QString& columnName)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    emit layoutAboutToBeChanged();
    auto column = static_cast<size_t>(indexForColumnName(columnName));
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
    Q_ASSERT(row < _nodeSelectedColumn.size());
    return _nodeSelectedColumn[row].toBool();
}

QString NodeAttributeTableModel::columnNameFor(size_t column) const
{
    Q_ASSERT(column < static_cast<size_t>(_columnNames.size()));
    return _columnNames.at(static_cast<int>(column));
}

void NodeAttributeTableModel::onAttributesChanged(const QStringList& added, const QStringList& removed)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    QSet<QString> columnNamesUnique(_columnNames.begin(), _columnNames.end());
    QSet<QString> addedUnique(added.begin(), added.end());
    QSet<QString> removedUnique(removed.begin(), removed.end());

    Q_ASSERT(added.isEmpty() || columnNamesUnique.intersect(addedUnique).isEmpty());

    // Ignore attribute names we aren't using (they may ncolumnNamesUniqueot be node attributes)
    auto filteredRemoved = columnNamesUnique.intersect(removedUnique).values();

    if(added.isEmpty() && filteredRemoved.isEmpty())
    {
        // There is no structural change to the table, but some roles' values
        // may have changed, so we need to update these individually
        while(!_columnsRequiringUpdates.empty())
        {
            auto attribute = _columnsRequiringUpdates.back();
            updateAttribute(attribute);
            _columnsRequiringUpdates.pop_back();
        }

        return;
    }

    // Any roles requiring an update will be taken care of en-masse, in onUpdateComplete
    _columnsRequiringUpdates.clear();

    for(const auto& name : filteredRemoved)
    {
        auto columnIndex = static_cast<size_t>(_columnNames.indexOf(name));
        onColumnRemoved(columnIndex);
        emit columnRemoved(columnIndex, name);
    }

    updateColumnNames();

    for(const auto& name : added)
    {
        const auto* attribute = _document->graphModel()->attributeByName(name);

        // We only care about node attributes, obviously
        if(attribute->elementType() != ElementType::Node || attribute->hasParameter())
            continue;

        auto columnIndex = static_cast<size_t>(_columnNames.indexOf(name));

        onColumnAdded(columnIndex);
        emit columnAdded(columnIndex, name);
    }

    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onAttributeValuesChanged(const QStringList& attributeNames)
{
    std::copy_if(attributeNames.begin(), attributeNames.end(), std::back_inserter(_columnsRequiringUpdates),
    [&](const auto& attributeName)
    {
        return u::contains(_columnNames, attributeName);
    });
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
    auto column = static_cast<size_t>(index.column());
    if(role == Qt::DisplayRole)
    {
        if(column >= _data.size())
            return {};

        const auto& dataColumn = _data.at(column);

        auto row = static_cast<size_t>(index.row());
        if(row >= dataColumn.size())
            return {};

        auto cachedValue = dataColumn.at(row);
        return cachedValue;
    }

    if(role == Roles::NodeSelectedRole && !_nodeSelectedColumn.empty())
    {
        auto row = static_cast<size_t>(index.row());
        return _nodeSelectedColumn.at(row);
    }
    return {};
}

void NodeAttributeTableModel::onSelectionChanged()
{
    updateColumn(Roles::NodeSelectedRole, _nodeSelectedColumn);
    emit selectionChanged();
}
