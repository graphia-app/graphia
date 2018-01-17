#include "colorvisualisationchannel.h"

#include <QObject>
#include <QCryptographicHash>

void ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._outerColor = _colorGradient.get(value);
}

void ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    if(value.isEmpty())
        return;

    QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
    hash.addData(value.toUtf8());
    auto result = hash.result();
    int hue = 0, lightness = 0;

    for(int i = 0; i < result.size(); i++)
    {
        int byte = result.at(i) + 128;
        if(i < result.size() / 2)
            hue = (hue + byte) % 255;
        else
            lightness = (lightness + byte) % 127;
    }

    lightness += 64;

    elementVisual._outerColor.setHsl(hue, 255, lightness);
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
    if(name == QLatin1String("gradient"))
        _colorGradient = ColorGradient(value);
}
