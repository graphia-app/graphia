#ifndef ATTRIBUTESTABLEMODEL_H
#define ATTRIBUTESTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QHash>

#include <vector>

class Attributes;

class AttributesTableModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList columnNames READ columnNames NOTIFY columnNamesChanged)

private:
    const Attributes* _rowAttributes;
    std::vector<int> _selectedRowIndexes;

    int _nextRole = Qt::UserRole + 1;
    QHash<int, QByteArray> _roleNames;

    QStringList columnNames() const;

public:
    explicit AttributesTableModel(Attributes* attributes);

    void setSelectedRowIndexes(std::vector<int>&& selectedRowIndexes);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const { return _roleNames; }

signals:
    void columnNamesChanged();
};

#endif // ATTRIBUTESTABLEMODEL_H
