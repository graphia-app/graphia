#include "usernodedatastablemodel.h"

#include "usernodedata.h"

UserNodeDataTableModel::UserNodeDataTableModel(UserNodeData* userNodeData) :
    QAbstractTableModel(),
    _userNodeData(userNodeData)
{
    _roleNames.insert(_nodeIdRole, "nodeId");
    _roleNames.insert(_nodeSelectedRole, "nodeSelected");

    connect(userNodeData, SIGNAL(userDataVectorAdded(const QString&)),
            this, SLOT(onUserDataVectorAdded(const QString&)));
}

void UserNodeDataTableModel::initialise(ISelectionManager* selectionManager)
{
    _selectionManager = selectionManager;
}

QStringList UserNodeDataTableModel::columnNames() const
{
    QStringList list;

    for(const auto& userDataVector : *_userNodeData)
        list.append(userDataVector.name());

    return list;
}

void UserNodeDataTableModel::onUserDataVectorAdded(const QString& name)
{
    _roleNames.insert(_nextRole, name.toUtf8());
    _nextRole++;

    emit columnNamesChanged();
}

int UserNodeDataTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numValues());
}

int UserNodeDataTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_userNodeData->numUserDataVectors());
}

QVariant UserNodeDataTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < rowCount() && role >= Qt::UserRole)
    {
        NodeId nodeId = _userNodeData->nodeIdForRowIndex(row);

        if(role == _nodeIdRole)
            return static_cast<int>(nodeId);
        else if(role == _nodeSelectedRole)
            return _selectionManager->nodeIsSelected(nodeId);

        return _userNodeData->value(row, _roleNames[role]);
    }

    return QVariant();
}

void UserNodeDataTableModel::onSelectionChanged()
{
    emit layoutChanged();
}
