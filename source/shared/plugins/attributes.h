#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "attribute.h"

#include "shared/utils/pair_iterator.h"

#include <QObject>
#include <QString>

#include <vector>

class Attributes : public QObject
{
    Q_OBJECT

private:
    std::vector<std::pair<QString, Attribute>> _attributes;
    QString _firstAttributeName;

protected:
    const QString firstAttributeName() const;

public:
    int numAttributes() const;
    int numValues() const;

    bool empty() const { return _attributes.empty(); }

    auto begin() const { return make_pair_second_iterator(_attributes.begin()); }
    auto end() const { return make_pair_second_iterator(_attributes.end()); }

    void add(const QString& name);
    void setValue(int index, const QString& name, const QString& value);
    QString value(int index, const QString& name) const;

signals:
    void attributeAdded(const QString& name);
};

#endif // ATTRIBUTES_H
