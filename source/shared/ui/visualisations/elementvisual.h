#ifndef ELEMENTVISUAL_H
#define ELEMENTVISUAL_H

#include "shared/utils/flags.h"

#include <QColor>

enum VisualFlags
{
    None     = 0x0,
    Selected = 0x1,
    NotFound = 0x2
};

struct ElementVisual
{
    float _size = -1.0f;
    QColor _outerColor;
    QColor _innerColor;
    QString _text;
    Flags<VisualFlags> _state = VisualFlags::None;
};

#endif // ELEMENTVISUAL_H
