#ifndef COLOR_H
#define COLOR_H

#include <QColor>
#include <QString>

namespace u
{
    QColor contrastingColor(const QColor& color);

    QColor colorForString(const QString& string);
} // namespace u

#endif // COLOR_H
