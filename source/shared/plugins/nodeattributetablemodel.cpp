#include "nodeattributetablemodel.h"

#include "usernodedata.h"

NodeAttributeTableModel::NodeAttributeTableModel() :
    QAbstractTableModel()
{
    _roleNames.insert(_nodeIdRole, "nodeId");
    _roleNames.insert(_nodeSelectedRole, "nodeSelected");
}

void NodeAttributeTableModel::initialise(ISelectionManager* selectionManager, IGraphModel* graphModel, UserNodeData* userNodeData)
{
    _selectionManager = selectionManager;
    _graphModel = graphModel;
    _userNodeData = userNodeData;

    refreshRoleNames();
    emit columnNamesChanged();

    auto modelQObject = dynamic_cast<const QObject*>(graphModel);
    connect(modelQObject, SIGNAL(attributeAdded(QString)),
            this, SLOT(onAttributeAdded(QString)));
    connect(modelQObject, SIGNAL(attributeRemoved(QString)),
            this, SLOT(onAttributeRemoved(QString)));
}

QStringList NodeAttributeTableModel::columnNames() const
{
    QStringList list;

    if(_showCalculatedAttributes)
        for(auto& attributeName : _graphModel->nodeAttributeNames())
            list.append(attributeName);
    else
        for(const auto& userDataVector : *_userNodeData)
            list.append(userDataVector.name());

    return list;
}

void NodeAttributeTableModel::refreshRoleNames()
{
    // Regenerate rolenames
    _nextRole = Qt::UserRole + 3;
    _roleNames.clear();
    _roleNames.insert(_nodeIdRole, "nodeId");
    _roleNames.insert(_nodeSelectedRole, "nodeSelected");

    for(const auto& name : _graphModel->nodeAttributeNames())
    {
        _roleNames.insert(_nextRole, name.toUtf8());
        _nextRole++;
    }
}

void NodeAttributeTableModel::showCalculatedAttributes(bool shouldShow)
{
    _showCalculatedAttributes = shouldShow;
    refreshRoleNames();
    emit columnNamesChanged();
}

void NodeAttributeTableModel::onAttributeAdded(QString name)
{
    // Recreate rolenames in the model if the attribute is new
    if(!u::contains(_roleNames.values(), name.toUtf8()))
    {
        refreshRoleNames();
        emit columnNamesChanged();
    }
}

void NodeAttributeTableModel::onAttributeRemoved(QString name)
{
    if(u::contains(_roleNames.values(), name.toUtf8()))
    {
        refreshRoleNames();
        emit columnNamesChanged();
    }
}

int NodeAttributeTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numValues());
}

int NodeAttributeTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(columnNames().size());
}

QVariant NodeAttributeTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < rowCount() && role >= Qt::UserRole && role < _nextRole)
    {
        NodeId nodeId = _userNodeData->nodeIdForRowIndex(row);

        if(role == _nodeIdRole)
            return static_cast<int>(nodeId);
        else if(role == _nodeSelectedRole)
            return _selectionManager->nodeIsSelected(nodeId);

        auto* attribute = _graphModel->attributeByName(_roleNames[role]);
        if(attribute != nullptr)
            return QVariant(attribute->stringValueOf(_userNodeData->nodeIdForRowIndex(row)));
    }

    return QVariant();
}

void NodeAttributeTableModel::onSelectionChanged()
{
    emit layoutChanged();
}
