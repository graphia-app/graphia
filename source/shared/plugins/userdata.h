#ifndef USERDATA_H
#define USERDATA_H

#include "userdatavector.h"

#include "shared/utils/pair_iterator.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <vector>

class UserData : public QObject
{
    Q_OBJECT

private:
    std::vector<std::pair<QString, UserDataVector>> _userDataVectors;
    QString _firstUserDataVectorName;

protected:
    const QString firstUserDataVectorName() const;

public:
    int numUserDataVectors() const;
    int numValues() const;

    bool empty() const { return _userDataVectors.empty(); }

    auto begin() const { return make_pair_second_iterator(_userDataVectors.begin()); }
    auto end() const { return make_pair_second_iterator(_userDataVectors.end()); }

    void add(const QString& name);
    void setValue(size_t index, const QString& name, const QString& value);
    QVariant value(size_t index, const QString& name) const;

signals:
    void userDataVectorAdded(const QString& name);
};

#endif // USERDATA_H
