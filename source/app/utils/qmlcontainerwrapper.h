#ifndef QMLCONTAINERWRAPPER_H
#define QMLCONTAINERWRAPPER_H

#include "shared/utils/utils.h"

#include <QAbstractListModel>
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QVariant>
#include <QHash>

#include <vector>
#include <map>
#include <set>
#include <memory>

// This whole class is substantially derived from the Qt QML Tricks library by Thomas Boutroue

class QmlContainerWrapperBase : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QmlContainerWrapperBase(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    {}

protected slots:
    virtual void onItemPropertyChanged() = 0;
};

template<class T> class QmlContainerWrapper : public QmlContainerWrapperBase
{
private:
    // We can either own the vector, or store a pointer to an external vector
    std::vector<T> _vector;
    std::vector<T>* _vectorPtr = nullptr;

    QMetaObject _metaObject;
    QMetaMethod _handler;
    QHash<int, QByteArray> _roles;
    std::map<int, int> _signalIndexToRole;

public:
    explicit QmlContainerWrapper(QObject* parent = nullptr) :
        QmlContainerWrapperBase(parent),
        _metaObject(T::staticMetaObject)
    {
        std::set<QByteArray> roleNamesBlacklist =
        {
            QByteArrayLiteral("id"),
            QByteArrayLiteral("index"),
            QByteArrayLiteral("class"),
            QByteArrayLiteral("model"),
            QByteArrayLiteral("modelData")
        };

        static const char* HANDLER = "onItemPropertyChanged()";
        _handler = metaObject()->method(metaObject()->indexOfMethod(HANDLER));

        _roles.insert(Qt::UserRole, QByteArrayLiteral("qtObject"));
        const int numProperties = _metaObject.propertyCount();

        for(int propertyIndex = 0, role = (Qt::UserRole + 1); propertyIndex < numProperties; propertyIndex++, role++)
        {
            QMetaProperty metaProperty = _metaObject.property(propertyIndex);
            const QByteArray propertyName = QByteArray(metaProperty.name());

            if(u::contains(roleNamesBlacklist, propertyName))
                continue;

            _roles.insert(role, propertyName);
            if(metaProperty.hasNotifySignal())
                _signalIndexToRole.emplace(metaProperty.notifySignalIndex(), role);
        }
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return static_cast<int>(vector().size());
    }

    QHash<int, QByteArray> roleNames() const
    {
        return _roles;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        auto row = index.row();

        QVariant variant;

        if(row < rowCount() && u::contains(_roles, role))
        {
            auto& item = vector().at(row);
            auto& roleName = _roles.value(role);
            variant.setValue(role != Qt::UserRole ? item.property(roleName) : "");
        }

        return variant;
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role)
    {
        auto row = index.row();

        if(row < rowCount() && u::contains(_roles, role))
        {
            auto& item = vector().at(row);
            auto& roleName = _roles.value(role);
            return item.setProperty(roleName, value);
        }

        return false;
    }

    const std::vector<T>& vector() const
    {
        if(_vectorPtr)
            return *_vectorPtr;

        return _vector;
    }

    std::vector<T>& vector()
    {
        if(_vectorPtr)
            return *_vectorPtr;

        return _vector;
    }

    void setVector(const std::vector<T>& vector)
    {
        beginResetModel();
        disconnectNotifiers();
        _vector = vector;
        _vectorPtr = nullptr;
        connectNotifiers();
        endResetModel();
    }

    void setVectorPtr(std::vector<T>* vectorPtr)
    {
        beginResetModel();
        disconnectNotifiers();
        _vector.clear();
        _vectorPtr = vectorPtr;
        connectNotifiers();
        endResetModel();
    }

private:
    void connectNotifiers()
    {
        for(auto& item : vector())
        {
            for(auto signalIndex : _signalIndexToRole)
            {
                QMetaMethod notifier = item.metaObject()->method(signalIndex.first);
                connect(&item, notifier, this, _handler, Qt::UniqueConnection);
            }
        }
    }

    void disconnectNotifiers()
    {
        for(auto& item : vector())
        {
            disconnect(this, nullptr, &item, nullptr);
            disconnect(&item, nullptr, this, nullptr);
        }
    }

    void onItemPropertyChanged()
    {
        int row = -1;
        int i = 0;

        for(auto& item : vector())
        {
            if(&item == sender())
            {
                row = i;
                break;
            }

            i++;
        }

        const int signalIndex = senderSignalIndex();
        const int role = _signalIndexToRole.at(signalIndex);

        if(row >= 0 && role >= 0)
        {
            QModelIndex index = QAbstractListModel::index(row, 0);
            emit dataChanged(index, index, {role});
        }
    }
};

#endif // QMLCONTAINERWRAPPER_H
