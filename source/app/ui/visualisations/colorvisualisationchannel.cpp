#include "colorvisualisationchannel.h"

#include <QObject>

void ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._color = _colorGradient.get(value);
}

void ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    if(value.isEmpty())
        return;

    auto hash = qHash(value);
    auto value1 = static_cast<int>(hash % 65535);
    auto value2 = static_cast<int>((hash >> 16) % 65535);

    elementVisual._color.setHsl(
        (value1 * 255) / 65535,       // Hue
        255,                          // Saturation
        ((value2 * 127) / 65535) + 64 // Lightness
        );
}

QString ColorVisualisationChannel::description(ElementType elementType, ValueType valueType) const
{
    auto elementTypeString = elementTypeAsString(elementType).toLower();

    switch(valueType)
    {
    case ValueType::Int:
    case ValueType::Float:
        return QString(QObject::tr("The attribute will be visualised by "
                          "continuously varying the colour of the %1 "
                          "using a gradient.")).arg(elementTypeString);

    case ValueType::String:
        return QString(QObject::tr("This visualisation will be applied by "
                          "assigning a colour to the %1 which represents "
                          "unique values of the attribute.")).arg(elementTypeString);

    default:
        break;
    }

    return {};
}

QVariantMap ColorVisualisationChannel::defaultParameters(ValueType valueType) const
{
    QVariantMap parameters;

    switch(valueType)
    {
    case ValueType::Int:
    case ValueType::Float:
        parameters.insert("gradient",
            R"("{
              \"0\":    \"Red\",
              \"0.66\": \"Yellow\",
              \"1\":    \"White\"
            }")");
        break;

    case ValueType::String:
    default:
        break;
    }

    return parameters;
}

void ColorVisualisationChannel::setParameter(const QString& name, const QString& value)
{
    if(name == "gradient")
        _colorGradient = value;
}
