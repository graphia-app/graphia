#ifndef TABLEPROXYMODEL_H
#define TABLEPROXYMODEL_H

#include <unordered_set>

#include <QSortFilterProxyModel>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QItemSelectionRange>
#include <QStandardItemModel>

class TableProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* headerModel READ headerModel CONSTANT)
    Q_PROPERTY(std::vector<int> hiddenColumns MEMBER _hiddenColumns WRITE setHiddenColumns)
    Q_PROPERTY(std::vector<int> columnOrder MEMBER _sourceColumnOrder WRITE setColumnOrder NOTIFY columnOrderChanged)
    Q_PROPERTY(int sortColumn MEMBER _sortColumn WRITE setSortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder MEMBER _sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

private:
    QStandardItemModel _headerModel;
    bool _showCalculatedColumns = false;
    QItemSelection _subSelection;
    std::unordered_set<int> _subSelectionRows;
    std::vector<int> _hiddenColumns;
    std::vector<int> _sourceColumnOrder;
    std::vector<int> _mappedColumnOrder;
    int _sortColumn = -1;
    Qt::SortOrder _sortOrder = Qt::DescendingOrder;
    enum Roles
    {
        SubSelectedRole = Qt::UserRole + 999
    };

    QAbstractItemModel* headerModel()
    {
        return &_headerModel;
    }
    void recalculateOrderMapping();
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;
public:
    QVariant data(const QModelIndex &index, int role) const override;
    static void registerQmlType()
    {
        static bool initialised = false;
        if(initialised)
            return;
        initialised = true;
        qmlRegisterType<TableProxyModel>(APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "TableProxyModel");
    }
    QHash<int, QByteArray> roleNames() const override
    {
        auto roleNames = sourceModel()->roleNames();
        roleNames.insert(Roles::SubSelectedRole, "subSelected");
        return roleNames;
    }
    TableProxyModel(QObject* parent = nullptr);
    Q_INVOKABLE void setSubSelection(QItemSelection subSelection, QItemSelection subDeselection);
    Q_INVOKABLE int mapToSourceRow(int proxyRow) const;
    Q_INVOKABLE int mapToSourceColumn(int proxyColumn) const;

    using QSortFilterProxyModel::mapToSource;

    Q_INVOKABLE QItemSelectionRange buildRowSelectionRange(int topLeft, int bottomRight);
    void setHiddenColumns(std::vector<int> hiddenColumns);
    void setColumnOrder(std::vector<int> columnOrder);
    void setSortColumn(int sortColumn);
    void setSortOrder(Qt::SortOrder sortColumn);
signals:
    void countChanged();
    void sortColumnChanged(int sortColumn);
    void sortOrderChanged(int sortColumn);

    void filterRoleNameChanged();
    void filterPatternSyntaxChanged();
    void filterPatternChanged();
    void filterValueChanged();

    void sortRoleNameChanged();
    void ascendingSortOrderChanged();

    void showCalculatedColumnsChanged();
    void columnOrderChanged();

public slots:
    void invalidateFilter();
};

static void initialiser()
{
    if(!QCoreApplication::instance()->startingUp())
    {
        QTimer::singleShot(0, [] { TableProxyModel::registerQmlType(); });
    }
    else
        TableProxyModel::registerQmlType();
}

Q_COREAPP_STARTUP_FUNCTION(initialiser)

#endif // TABLEPROXYMODEL_H
