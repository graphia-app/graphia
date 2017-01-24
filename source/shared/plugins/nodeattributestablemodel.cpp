#include "nodeattributestablemodel.h"

#include "nodeattributes.h"

NodeAttributesTableModel::NodeAttributesTableModel(NodeAttributes* attributes) :
    QAbstractTableModel(),
    _nodeAttributes(attributes)
{
    _roleNames.insert(_nodeIdRole, "nodeId");
    _roleNames.insert(_nodeSelectedRole, "nodeSelected");

    connect(attributes, SIGNAL(attributeAdded(const QString&)),
            this, SLOT(onAttributeAdded(const QString&)));
}

void NodeAttributesTableModel::initialise(ISelectionManager* selectionManager)
{
    _selectionManager = selectionManager;
}

QStringList NodeAttributesTableModel::columnNames() const
{
    QStringList list;

    for(const auto& rowAttribute : *_nodeAttributes)
        list.append(rowAttribute.name());

    return list;
}

void NodeAttributesTableModel::onAttributeAdded(const QString& name)
{
    _roleNames.insert(_nextRole, name.toUtf8());
    _nextRole++;

    emit columnNamesChanged();
}

int NodeAttributesTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_nodeAttributes->numValues());
}

int NodeAttributesTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_nodeAttributes->numAttributes());
}

QVariant NodeAttributesTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < rowCount() && role >= Qt::UserRole)
    {
        NodeId nodeId = _nodeAttributes->nodeIdForRowIndex(row);

        if(role == _nodeIdRole)
            return static_cast<int>(nodeId);
        else if(role == _nodeSelectedRole)
            return _selectionManager->nodeIsSelected(nodeId);

        return _nodeAttributes->value(row, _roleNames[role]);
    }

    return QVariant();
}

int NodeAttributesTableModel::modelIndexRow(const QModelIndex& index) const
{
    return index.row();
}

void NodeAttributesTableModel::onSelectionChanged()
{
    emit layoutChanged();
}
