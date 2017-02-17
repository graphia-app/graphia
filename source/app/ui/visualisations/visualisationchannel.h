#ifndef VISUALISATIONCHANNEL_H
#define VISUALISATIONCHANNEL_H

#include "ui/visualisations/elementvisual.h"
#include "transform/fieldtype.h"

#include <QString>

class VisualisationChannel
{
public:
    virtual ~VisualisationChannel() {}

    // Numerical value is normalised
    virtual void apply(double, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }
    virtual void apply(const QString&, ElementVisual&) const { Q_ASSERT(!"apply not implemented"); }

    virtual bool supports(FieldType) = 0;
};

#endif // VISUALISATIONCHANNEL_H
