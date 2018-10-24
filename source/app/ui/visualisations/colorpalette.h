#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QString>
#include <QColor>

#include <vector>

class ColorPalette
{
private:
    std::vector<QColor> _colors;
    QColor _otherColor;

public:
    ColorPalette() = default;
    ColorPalette(const ColorPalette&) = default;
    ColorPalette& operator=(const ColorPalette&) = default;
    explicit ColorPalette(const QString& descriptor);

    QColor get(const QString& value) const;
};

#endif // COLORPALETTE_H
