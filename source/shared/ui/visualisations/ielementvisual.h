#ifndef IELEMENTVISUAL_H
#define IELEMENTVISUAL_H

#include "shared/utils/flags.h"

#include <QColor>

enum VisualFlags
{
    None     = 0x0,
    Selected = 0x1,
    NotFound = 0x2
};

struct IElementVisual
{
    virtual ~IElementVisual() {}

    virtual float size() const = 0;
    virtual QColor outerColor() const = 0;
    virtual QColor innerColor() const = 0;
    virtual QString text() const = 0;
    virtual Flags<VisualFlags> state() const = 0;
};

#endif // IELEMENTVISUAL_H
