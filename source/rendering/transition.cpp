#include "transition.h"

#include "../maths/interpolation.h"
#include "../utils/utils.h"

#include <QDebug>

bool Transition::update(float dTime)
{
    if(!active())
        return false;

    _elapsed += dTime;

    float f = u::clamp(0.0f, 1.0f, _elapsed / _duration);

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
        for(auto& finishedFunction : _finishedFunctions)
            finishedFunction();

        return false;
    }

    return true;
}

