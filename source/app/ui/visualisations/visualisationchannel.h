#ifndef VISUALISATIONCHANNEL_H
#define VISUALISATIONCHANNEL_H

#include "ui/visualisations/elementvisual.h"
#include "attributes/valuetype.h"
#include "graph/elementtype.h"

#include <QString>
#include <QVariantMap>

class VisualisationChannel
{
public:
    virtual ~VisualisationChannel() = default;

    // Numerical value is normalised
    virtual void apply(double, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }
    virtual void apply(const QString&, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }

    virtual bool supports(ValueType) const = 0;
    virtual bool requiresNormalisedValue() const { return true; }

    virtual QString description(ElementType, ValueType) const { return {}; }

    virtual QVariantMap defaultParameters(ValueType) const { return {}; }
    virtual void setParameter(const QString& /*name*/, const QString& /*value*/) {}
};

#endif // VISUALISATIONCHANNEL_H
