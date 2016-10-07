#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "attribute.h"

#include <QObject>
#include <QString>

#include <vector>

class IGraphModel;

class Attributes : public QObject
{
    Q_OBJECT

private:
    std::vector<Attribute> _attributes;

    Attribute& attributeByName(const QString& name);
    const Attribute& attributeByName(const QString& name) const;

public:
    int size() const;
    void reserve(int size);

    bool empty() const { return _attributes.empty(); }

    std::vector<Attribute>::const_iterator begin() const { return _attributes.begin(); }
    std::vector<Attribute>::const_iterator end() const { return _attributes.end(); }

    const QString& firstAttributeName() const { return _attributes.front().name(); }

    void add(const QString& name);
    void setValue(int index, const QString& name, const QString& value);
    QString value(int index, const QString& name) const;

signals:
    void attributeAdded(const QString& name);
};

#endif // ATTRIBUTES_H
