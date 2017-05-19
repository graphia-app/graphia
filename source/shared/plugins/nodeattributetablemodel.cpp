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
    connect(modelQObject, SIGNAL(attributeAdded(const QString&)),
            this, SLOT(onAttributeAdded(const QString&)));
    connect(modelQObject, SIGNAL(attributeRemoved(const QString&)),
            this, SLOT(onAttributeRemoved(const QString&)));
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

void NodeAttributeTableModel::refreshRoleNames()
{
    // Regenerate rolenames
    // +3 because we added _nodeIdRole + _nodeSelectedRole on init
    _nextRole = Qt::UserRole + 3;
    _roleNames.clear();
    _roleNames.insert(_nodeIdRole, "nodeId");
    _roleNames.insert(_nodeSelectedRole, "nodeSelected");

    for(const auto& name : columnNames())
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

void NodeAttributeTableModel::onAttributeAdded(const QString& name)
{
    // Recreate rolenames in the model if the attribute is new
    if(!u::contains(_roleNames.values(), name.toUtf8()))
    {
        refreshRoleNames();
        emit columnNamesChanged();
    }
}

void NodeAttributeTableModel::onAttributeRemoved(const QString& name)
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
            return attribute->valueOf(_userNodeData->nodeIdForRowIndex(row));
    }

    return {};
}

void NodeAttributeTableModel::onSelectionChanged()
{
    emit layoutChanged();
}
