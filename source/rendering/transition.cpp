#include "transition.h"

#include "../maths/interpolation.h"
#include "../utils/utils.h"

#include <QDebug>

Transition::Transition() :
    _type(Type::None), _lastTime(0.0f),
    _duration(0.0f), _elapsed(0.0f),
    _function([](float){})
{}



bool Transition::update(float time)
{
    if(!active())
        return false;

    if(_lastTime <= 0.0f)
        _lastTime = time;

    _elapsed += time - _lastTime;
    _lastTime = time;

    float f = Utils::clamp(0.0f, 1.0f, _elapsed / _duration);

    switch(_type)
    {
    case Type::Linear:
    default:
        f = Interpolation::linear(0.0f, 1.0f, f);
        break;

    case Type::EaseInEaseOut:
        f = Interpolation::easeInEaseOut(0.0f, 1.0f, f);
        break;

    case Type::Power:
        f = Interpolation::power(0.0f, 1.0f, f);
        break;

    case Type::InversePower:
        f = Interpolation::inversePower(0.0f, 1.0f, f);
        break;
    }

    _function(f);

    if(!active())
    {
        for(auto finishedFunction : _finishedFunctions)
            finishedFunction();

        return false;
    }

    return true;
}

