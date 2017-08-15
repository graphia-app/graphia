#ifndef USERDATA_H
#define USERDATA_H

#include "userdatavector.h"

#include "shared/utils/pair_iterator.h"
#include "shared/loading/progressfn.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include "thirdparty/json/json_helper.h"

#include <vector>

class UserData : public QObject
{
    Q_OBJECT

private:
    // This is not a map because the data needs to be ordered
    std::vector<std::pair<QString, UserDataVector>> _userDataVectors;
    int _numValues = 0;

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

    json save(const ProgressFn& progressFn) const;
    bool load(const json& jsonObject, const ProgressFn& progressFn);

signals:
    void userDataVectorAdded(const QString& name);
};

#endif // USERDATA_H
