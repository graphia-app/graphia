#include "nodeattributestablemodel.h"

#include "nodeattributes.h"

NodeAttributesTableModel::NodeAttributesTableModel(NodeAttributes* attributes) :
    QAbstractTableModel(),
    _nodeAttributes(attributes)
{
    connect(attributes, &Attributes::attributeAdded,
    [this](const QString& name)
    {
        _roleNames.insert(_nextRole, name.toUtf8());
        _nextRole++;

        emit columnNamesChanged();
    });
}

void NodeAttributesTableModel::setSelectedNodes(const NodeIdSet& selectedNodeIds)
{
    emit layoutAboutToBeChanged();
    _selectedNodeIds.clear();
    std::copy(selectedNodeIds.begin(), selectedNodeIds.end(), std::back_inserter(_selectedNodeIds));
    emit layoutChanged();
}

QStringList NodeAttributesTableModel::columnNames() const
{
    QStringList list;

    for(const auto& rowAttribute : *_nodeAttributes)
        list.append(rowAttribute.name());

    return list;
}

int NodeAttributesTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_selectedNodeIds.size());
}

int NodeAttributesTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_nodeAttributes->size());
}

QVariant NodeAttributesTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < rowCount() && role >= Qt::UserRole)
    {
        return _nodeAttributes->valueByNodeId(_selectedNodeIds.at(index.row()), _roleNames[role]);
    }

    return QVariant();
}
