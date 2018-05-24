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
        TransformCategoryRole = Qt::UserRole + 1
    };

    const GraphModel* _graphModel = nullptr;

public:
    AvailableTransformsModel() = default;
    explicit AvailableTransformsModel(const GraphModel& graphModel,
                             QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant get(const QModelIndex& index) const;

    QHash<int, QByteArray> roleNames() const override;
};

#endif // AVAILABLETRANSFORMSMODEL_H
