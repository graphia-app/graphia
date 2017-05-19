#ifndef ELEMENTVISUAL_H
#define ELEMENTVISUAL_H

#include "shared/ui/visualisations/ielementvisual.h"

struct ElementVisual : IElementVisual
{
    float _size = -1.0f;
    QColor _outerColor;
    QColor _innerColor;
    QString _text;
    Flags<VisualFlags> _state = VisualFlags::None;

    float size() const { return _size; }
    QColor outerColor() const { return _outerColor; }
    QColor innerColor() const { return _innerColor; }
    QString text() const { return _text; }
    Flags<VisualFlags> state() const { return _state; }
};

#endif // ELEMENTVISUAL_H
