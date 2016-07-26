#include "attributestablemodel.h"

#include "correlationplugin.h"

AttributesTableModel::AttributesTableModel(CorrelationPluginInstance* correlationPluginInstance) :
    _correlationPluginInstance(correlationPluginInstance)
{
    connect(correlationPluginInstance, &CorrelationPluginInstance::selectionChanged,
            this, &AttributesTableModel::onSelectionChanged);
}

void AttributesTableModel::initialise()
{
    int role = Qt::UserRole;
    for(auto& rowAttribute : _correlationPluginInstance->rowAttributes())
        _roleNames.insert(role++, rowAttribute._name.toUtf8());

    emit columnNamesChanged();
}

QStringList AttributesTableModel::columnNames() const
{
    QStringList list;

    for(auto& rowAttribute : _correlationPluginInstance->rowAttributes())
        list.append(rowAttribute._name);

    return list;
}

int AttributesTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_selectedRowIndexes.size());
}

int AttributesTableModel::columnCount(const QModelIndex&) const
{
    return static_cast<int>(_correlationPluginInstance->rowAttributes().size());
}

QVariant AttributesTableModel::data(const QModelIndex& index, int role) const
{
    if(role >= Qt::UserRole)
    {
        int rowIndex = _selectedRowIndexes.at(index.row());
        return _correlationPluginInstance->rowAttributeValue(rowIndex, _roleNames[role]);
    }

    return QVariant();
}

void AttributesTableModel::onSelectionChanged(const ISelectionManager* selectionManager)
{
    emit layoutAboutToBeChanged();

    const auto& selectedNodes = selectionManager->selectedNodes();

    _selectedRowIndexes.reserve(selectedNodes.size());
    _selectedRowIndexes.clear();

    for(auto& nodeId : selectedNodes)
    {
        int rowIndex = _correlationPluginInstance->rowIndexForNodeId(nodeId);
        _selectedRowIndexes.emplace_back(rowIndex);
    }

    emit layoutChanged();
}
