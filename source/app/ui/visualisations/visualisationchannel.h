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
    // The return value indicates whether or not application replaced an existing visualisation
    virtual bool apply(double, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }
    virtual bool apply(const QString&, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }

    virtual bool supports(FieldType) const = 0;
    virtual bool requiresNormalisedValue() const { return true; }

    virtual QString description(ElementType, FieldType) const { return {}; }
};

#endif // VISUALISATIONCHANNEL_H
