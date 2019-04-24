#ifndef IELEMENTVISUAL_H
#define IELEMENTVISUAL_H

#include "shared/utils/flags.h"

#include <QColor>

enum VisualFlags
{
    None          = 0x0,
    Selected      = 0x1,
    Unhighlighted = 0x2
};

struct IElementVisual
{
    IElementVisual() = default;
    IElementVisual(const IElementVisual&) = default;
    IElementVisual(IElementVisual&&) = default;
    IElementVisual& operator=(const IElementVisual&) = default;
    IElementVisual& operator=(IElementVisual&&) = default;

    virtual ~IElementVisual() = default;

    virtual float size() const = 0;
    virtual QColor outerColor() const = 0;
    virtual QColor innerColor() const = 0;
    virtual QString text() const = 0;
    virtual Flags<VisualFlags> state() const = 0;
};

#endif // IELEMENTVISUAL_H
