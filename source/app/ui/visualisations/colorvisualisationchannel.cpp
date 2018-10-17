#include "colorvisualisationchannel.h"

#include <QObject>

void ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._outerColor = _colorGradient.get(value);
}

void ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    if(value.isEmpty())
        return;

    elementVisual._outerColor = _colorPalette.get(value);
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
        parameters.insert(QStringLiteral("gradient"),
            R"("{
              \"0\":    \"White\",
              \"0.33\": \"Yellow\",
              \"1\":    \"Red\"
            }")");
        break;

    case ValueType::String:
        parameters.insert(QStringLiteral("palette"),
            R"("[
              \"#199311\",
              \"#FF7700\",
              \"#8126C0\",
              \"#1DF3F3\",
              \"#FFEE33\",
              \"#FD1111\",
              \"#DDDDDD\",
              \"#FF69B4\",
              \"#003BFF\",
              \"#8AFF1E\",
              \"#9D2323\",
              \"#798FF0\",
              \"#111111\"
            ]")");
        break;

    default:
        break;
    }

    return parameters;
}

void ColorVisualisationChannel::setParameter(const QString& name, const QString& value)
{
    if(name == QLatin1String("gradient"))
        _colorGradient = ColorGradient(value);
    else if(name == QLatin1String("palette"))
        _colorPalette = ColorPalette(value);
}
