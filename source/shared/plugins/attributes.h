#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "attribute.h"

#include "shared/utils/pair_iterator.h"

#include <QObject>
#include <QString>

class Attributes : public QObject
{
    Q_OBJECT

private:
    std::map<QString, Attribute> _attributes;
    QString _firstAttributeName;

public:
    int size() const;

    bool empty() const { return _attributes.empty(); }

    auto begin() const { return make_map_value_iterator(_attributes.begin()); }
    auto end() const { return make_map_value_iterator(_attributes.end()); }

    const QString& firstAttributeName() const { return _firstAttributeName; }

    void add(const QString& name);
    void setValue(int index, const QString& name, const QString& value);
    QString value(int index, const QString& name) const;

signals:
    void attributeAdded(const QString& name);
};

#endif // ATTRIBUTES_H
