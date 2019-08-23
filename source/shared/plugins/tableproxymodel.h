#ifndef TABLEPROXYMODEL_H
#define TABLEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QItemSelectionRange>

class TableProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QList<int> hiddenColumns MEMBER _hiddenColumns WRITE setHiddenColumns)

private:
    bool _showCalculatedColumns = false;
    QModelIndexList _subSelection;
    QList<int> _hiddenColumns;
    enum Roles
    {
        SubSelectedRole = Qt::UserRole + 999
    };

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
    Q_INVOKABLE void setSubSelection(QModelIndexList subSelection);
    Q_INVOKABLE int mapToSourceRow(int proxyRow) const;

    using QSortFilterProxyModel::mapToSource;

    Q_INVOKABLE QItemSelectionRange buildRowSelectionRange(int topLeft, int bottomRight);
    void setHiddenColumns(QList<int> hiddenColumns);
signals:
    void countChanged();

    void filterRoleNameChanged();
    void filterPatternSyntaxChanged();
    void filterPatternChanged();
    void filterValueChanged();

    void sortRoleNameChanged();
    void ascendingSortOrderChanged();

    void showCalculatedColumnsChanged();

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
