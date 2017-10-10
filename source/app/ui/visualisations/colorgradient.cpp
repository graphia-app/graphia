#include "colorgradient.h"

#include "shared/utils/utils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <algorithm>

ColorGradient::ColorGradient(const QString& descriptor)
{
    QJsonParseError error{0, QJsonParseError::ParseError::NoError};
    auto jsonDocument = QJsonDocument::fromJson(descriptor.toUtf8(), &error);

    if(jsonDocument.isNull())
    {
        qDebug() << "ColorGradient failed to parse:" << error.errorString() << descriptor;
        return;
    }

    if(!jsonDocument.isObject())
    {
        qDebug() << "ColorGradient is not an object";
        return;
    }

    auto jsonObject = jsonDocument.object();
    const auto keys = jsonObject.keys();

    for(const auto& key : keys)
    {
        auto value = key.toDouble();
        auto colorString = jsonObject.value(key).toString();

        _stops.emplace_back(Stop{value, colorString});
    }

    std::sort(_stops.begin(), _stops.end());
}

QColor ColorGradient::get(double value) const
{
    if(_stops.empty())
        return {};

    if(_stops.size() == 1)
        return _stops[0]._color;

    for(size_t i = 0; i < _stops.size() - 1; i++)
    {
        const auto& from = _stops[i];
        const auto& to =   _stops[i + 1];

        if(value >= from._value && value < to._value)
        {
            auto nv = u::normalise(from._value, to._value, value);
            auto const& fc = from._color;
            auto const& tc = to._color;

            auto rd = tc.redF()   - fc.redF();
            auto gd = tc.greenF() - fc.greenF();
            auto bd = tc.blueF()  - fc.blueF();

            QColor blend;
            blend.setRedF(   fc.redF()   + (nv * rd));
            blend.setGreenF( fc.greenF() + (nv * gd));
            blend.setBlueF(  fc.blueF()  + (nv * bd));
            return blend;
        }
    }

    if(value >= _stops.back()._value)
        return _stops.back()._color;

    return _stops.front()._color;
}
