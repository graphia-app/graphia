#include "transition.h"

#include "../maths/interpolation.h"
#include "../utils.h"

#include <QDebug>

Transition::Transition() :
    _type(Type::None), lastTime(0.0f),
    _duration(0.0f), elapsed(0.0f),
    _function([](float){})
{}

void Transition::start(float duration, Transition::Type type, std::function<void (float)> function)
{
    lastTime = -1.0f;
    _duration = duration;
    elapsed = 0.0f;
    _type = type;
    _function = function;
}

bool Transition::update(float time)
{
    if(finished())
        return true;

    if(lastTime <= 0.0f)
        lastTime = time;

    elapsed += time - lastTime;
    lastTime = time;

    float f = Utils::clamp(0.0f, 1.0f, elapsed / _duration);

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

    return finished();
}

