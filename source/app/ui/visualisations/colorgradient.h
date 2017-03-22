#ifndef COLORGRADIENT_H
#define COLORGRADIENT_H

#include <QString>
#include <QColor>

#include <vector>

class ColorGradient
{
private:
    struct Stop
    {
        double _value;
        QColor _color;

        bool operator<(const Stop& other) const
        {
            if(_value < other._value)
                return true;
            else if(_value == other._value)
                return _color.rgb() < other._color.rgb();

            return false;
        }
    };

    std::vector<Stop> _stops;

public:
    ColorGradient() = default;
    ColorGradient(const ColorGradient&) = default;
    ColorGradient& operator=(const ColorGradient&) = default;
    ColorGradient(const QString& descriptor);

    QColor get(double value) const;
};

#endif // COLORGRADIENT_H
