#ifndef ELEMENTVISUAL_H
#define ELEMENTVISUAL_H

#include <QFlags>
#include <QColor>

enum VisualFlags
{
    None     = 0x0,
    Selected = 0x1,
    NotFound = 0x2
};

Q_DECLARE_FLAGS(VisualState, VisualFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(VisualState)

struct ElementVisual
{
    float _size = -1.0f;
    QColor _outerColor;
    QColor _innerColor;
    QString _text;
    VisualState _state;
};

#endif // ELEMENTVISUAL_H
