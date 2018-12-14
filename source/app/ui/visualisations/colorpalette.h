#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QString>
#include <QColor>

#include <vector>
#include <map>

class ColorPalette
{
private:
    std::map<QString, QColor> _fixedColors;
    std::vector<QColor> _colors;
    QColor _defaultColor;

public:
    ColorPalette() = default;
    ColorPalette(const ColorPalette&) = default;
    ColorPalette& operator=(const ColorPalette&) = default;
    explicit ColorPalette(const QString& descriptor);

    QColor get(const QString& value, int index) const;
};

#endif // COLORPALETTE_H
