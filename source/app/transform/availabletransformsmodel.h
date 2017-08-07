#ifndef AVAILABLETRANSFORMSMODEL_H
#define AVAILABLETRANSFORMSMODEL_H

#include <QStringList>
#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

class GraphModel;

class AvailableTransformsModel : public QAbstractListModel
{
    Q_OBJECT

private:
    QStringList _transformNames;

    enum Roles
    {
        TransformTypeRole = Qt::UserRole + 1
    };

    const GraphModel* _graphModel = nullptr;

public:
    AvailableTransformsModel() = default;
    AvailableTransformsModel(const GraphModel& graphModel,
                             QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Q_INVOKABLE QVariant get(const QModelIndex& index) const;

    QHash<int, QByteArray> roleNames() const;
};

#endif // AVAILABLETRANSFORMSMODEL_H
