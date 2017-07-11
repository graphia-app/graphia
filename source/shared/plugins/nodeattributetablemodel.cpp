#include "nodeattributetablemodel.h"

#include "usernodedata.h"

NodeAttributeTableModel::NodeAttributeTableModel() :
    QAbstractTableModel()
{}

void NodeAttributeTableModel::initialise(ISelectionManager* selectionManager, IGraphModel* graphModel, UserNodeData* userNodeData)
{
    _selectionManager = selectionManager;
    _graphModel = graphModel;
    _userNodeData = userNodeData;

    updateRoleNames();

    auto modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributeAdded(const QString&)),
            this, SLOT(onAttributeAdded(const QString&)));
    connect(modelQObject, SIGNAL(attributeRemoved(const QString&)),
            this, SLOT(onAttributeRemoved(const QString&)));

    auto graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
    connect(graphQObject, SIGNAL(graphChanged(const Graph*, bool)),
            this, SLOT(onGraphChanged(const Graph*, bool)));
}

QStringList NodeAttributeTableModel::columnNames() const
{
    QStringList list;

    for(const auto& userDataVector : *_userNodeData)
        list.append(userDataVector.name());

    if(_showCalculatedAttributes)
    {
        for(auto& attributeName : _graphModel->attributeNames(ElementType::Node))
        {
            if(!u::contains(list, attributeName))
                list.append(attributeName);
        }
    }

    return list;
}

void NodeAttributeTableModel::update()
{
    //FIXME depending on the precise reasons for calling update(), it isn't
    // necessarily required to regenerate all the data from scratch. e.g.
    // if it's in response to a selection change, on the NodeSelectedRole
    // actually needs to change, and probably begin/endResetModel can be
    // avoided too

    _updatedData.clear();

    for(int roleNum = 0; roleNum < _roleNames.size(); roleNum++)
    {
        int role = Qt::UserRole + 1 + roleNum;

        for(int row = 0; row < rowCount(); row++)
        {
            NodeId nodeId = _userNodeData->nodeIdForRowIndex(row);

            if(!_graphModel->graph().containsNodeId(nodeId))
            {
                // The graph doesn't necessarily have a node for every row since
                // it may have been transformed, leaving empty rows
                _updatedData.append(QVariant());
                continue;
            }

            if(role == Roles::NodeIdRole)
                _updatedData.append(static_cast<int>(nodeId));
            else if(role == Roles::NodeSelectedRole)
                _updatedData.append(_selectionManager->nodeIsSelected(nodeId));
            else
            {
                auto* attribute = _graphModel->attributeByName(_roleNames[role]);
                if(attribute != nullptr)
                    _updatedData.append(attribute->valueOf(nodeId));
            }
        }
    }

    // Notify the main thread that the data has changed
    QMetaObject::invokeMethod(this, "onUpdateComplete");
}

void NodeAttributeTableModel::onUpdateComplete()
{
    beginResetModel();
    _cachedData = std::move(_updatedData);
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
    for(const auto& name : columnNames())
    {
        _roleNames.insert(nextRole, name.toUtf8());
        nextRole++;
    }

    _columnCount = columnNames().size();

    emit columnNamesChanged();
}

void NodeAttributeTableModel::showCalculatedAttributes(bool shouldShow)
{
    _showCalculatedAttributes = shouldShow;
    updateRoleNames();
}

void NodeAttributeTableModel::onAttributeAdded(const QString& name)
{
    // Recreate rolenames in the model if the attribute is new
    if(!u::contains(_roleNames.values(), name.toUtf8()))
        updateRoleNames();
}

void NodeAttributeTableModel::onAttributeRemoved(const QString& name)
{
    if(u::contains(_roleNames.values(), name.toUtf8()))
        updateRoleNames();
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
    int cacheIndex = ((role - Qt::UserRole - 1) * rowCount()) + index.row();
    Q_ASSERT(cacheIndex < _cachedData.size());
    auto cachedValue = _cachedData.at(cacheIndex);

    return cachedValue;
}

void NodeAttributeTableModel::onSelectionChanged()
{
    update();
}
