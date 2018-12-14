#ifndef VISUALISATIONCHANNEL_H
#define VISUALISATIONCHANNEL_H

#include "ui/visualisations/elementvisual.h"
#include "shared/attributes/valuetype.h"
#include "shared/graph/elementtype.h"

#include <QString>
#include <QVariantMap>

#include <vector>
#include <map>

class VisualisationInfo;

class VisualisationChannel
{
public:
    virtual ~VisualisationChannel() = default;

    // Numerical value is normalised
    virtual void apply(double, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }
    virtual void apply(const QString&, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }

    virtual bool supports(ValueType) const = 0;
    virtual bool requiresNormalisedValue() const { return true; }
    virtual bool requiresRange() const { return true; }

    virtual void findErrors(VisualisationInfo&) const {}

    virtual QString description(ElementType, ValueType) const { return {}; }

    virtual void reset();
    virtual QVariantMap defaultParameters(ValueType) const { return {}; }
    virtual void setParameter(const QString& /*name*/, const QString& /*value*/) {}

    const std::vector<QString>& values() const { return _values; }
    void addValue(const QString& value);
    int indexOf(const QString& value) const;

private:
    std::vector<QString> _values;
    std::map<QString, int> _valueIndexMap;
};

#endif // VISUALISATIONCHANNEL_H
