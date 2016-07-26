#ifndef ATTRIBUTESTABLEMODEL_H
#define ATTRIBUTESTABLEMODEL_H

#include "shared/ui/iselectionmanager.h"
#include "shared/graph/elementid.h"

#include <QAbstractTableModel>
#include <QStringList>

#include <vector>

class CorrelationPluginInstance;

class AttributesTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)

private:
    CorrelationPluginInstance* _correlationPluginInstance;
    std::vector<int> _selectedRowIndexes;
    QHash<int, QByteArray> _roleNames;

    QStringList columnNames() const;

public:
    explicit AttributesTableModel(CorrelationPluginInstance* correlationPluginInstance);

    void initialise();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

private slots:
    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void columnNamesChanged();
};

#endif // ATTRIBUTESTABLEMODEL_H
