#ifndef ELEMENTVISUAL_H
#define ELEMENTVISUAL_H

#include "shared/ui/visualisations/ielementvisual.h"

struct ElementVisual : IElementVisual
{
    double _size = -1.0f;
    QColor _outerColor;
    QColor _innerColor;
    QString _text;
    Flags<VisualFlags> _state = VisualFlags::None;

    float size() const override { return _size; }
    QColor outerColor() const override { return _outerColor; }
    QColor innerColor() const override { return _innerColor; }
    QString text() const override { return _text; }
    Flags<VisualFlags> state() const override { return _state; }
};

#endif // ELEMENTVISUAL_H
