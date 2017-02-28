#include "colorvisualisationchannel.h"

#include <QObject>

bool ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    bool alreadySet = elementVisual._color.isValid();

    QColor from(Qt::red);
    QColor to(Qt::yellow);
    QColor diff;

    auto redDiff = to.redF() -     from.redF();
    auto greenDiff = to.greenF() - from.greenF();
    auto blueDiff = to.blueF() -   from.blueF();

    elementVisual._color.setRedF(from.redF() +     (value * redDiff));
    elementVisual._color.setGreenF(from.greenF() + (value * greenDiff));
    elementVisual._color.setBlueF(from.blueF() +   (value * blueDiff));

    return alreadySet;
}

bool ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    bool alreadySet = elementVisual._color.isValid();

    if(value.isEmpty())
        return false;

    auto hash = qHash(value);
    int value1 = static_cast<int>(hash % 65535);
    int value2 = static_cast<int>((hash >> 16) % 65535);

    elementVisual._color.setHsl(
        (value1 * 255) / 65535,       // Hue
        255,                          // Saturation
        ((value2 * 127) / 65535) + 64 // Lightness
        );

    return alreadySet;
}

QString ColorVisualisationChannel::description(ElementType elementType, FieldType fieldType) const
{
    auto elementTypeString = elementTypeAsString(elementType).toLower();

    switch(fieldType)
    {
    case FieldType::Int:
    case FieldType::Float:
        return QString(QObject::tr("The attribute will be visualised by "
                          "continuously varying the colour of the %1 "
                          "using a gradient.")).arg(elementTypeString);

    case FieldType::String:
        return QString(QObject::tr("This visualisation will be applied by "
                          "assigning a colour to the %1 which represents "
                          "unique values of the attribute.")).arg(elementTypeString);

    default:
        break;
    }

    return {};
}
