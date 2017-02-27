#ifndef VISUALISATIONCHANNEL_H
#define VISUALISATIONCHANNEL_H

#include "ui/visualisations/elementvisual.h"
#include "transform/fieldtype.h"
#include "graph/elementtype.h"

#include <QString>

class VisualisationChannel
{
public:
    virtual ~VisualisationChannel() = default;

    // Numerical value is normalised
    virtual void apply(double, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }
    virtual void apply(const QString&, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }

    virtual bool supports(FieldType) const = 0;
    virtual bool requiresNormalisedValue() const { return true; }

    virtual QString description(ElementType, FieldType) const { return {}; }
};

#endif // VISUALISATIONCHANNEL_H
