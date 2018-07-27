#ifndef USERDATA_H
#define USERDATA_H

#include "userdatavector.h"

#include "shared/utils/pair_iterator.h"
#include "shared/loading/progressfn.h"

#include <QString>
#include <QVariant>
#include <QSet>

#include "thirdparty/json/json_helper.h"

#include <vector>

class UserData
{
private:
    // This is not a map because the data needs to be ordered
    std::vector<std::pair<QString, UserDataVector>> _userDataVectors;
    std::vector<QString> _vectorNames;
    int _numValues = 0;

public:
    QString firstUserDataVectorName() const;

    int numUserDataVectors() const;
    int numValues() const;

    bool empty() const { return _userDataVectors.empty(); }

    std::vector<QString> vectorNames() const;

    auto begin() const { return _userDataVectors.begin(); }
    auto end() const { return _userDataVectors.end(); }

    UserDataVector& add(QString name);
    void setValue(size_t index, const QString& name, const QString& value);
    QVariant value(size_t index, const QString& name) const;

    json save(const ProgressFn& progressFn) const;
    bool load(const json& jsonObject, const ProgressFn& progressFn);
};

#endif // USERDATA_H
