#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>

#include <functional>

class Transition : public QObject
{
    Q_OBJECT
public:
    Transition() : _function([](float) {}) {}

    enum class Type
    {
        None,
        Linear,
        EaseInEaseOut,
        Power,
        InversePower
    };

private:
    Type _type = Type::None;
    float _lastTime = 0.0f;
    float _duration = 0.0f;
    float _elapsed = 0.0f;
    std::function<void(float)> _function;
    std::vector<std::function<void()>> _finishedFunctions;

public:
    template<typename... Args> void start(float duration, Type type,
                                          std::function<void(float)> function,
                                          Args... finishedFunctions)
    {
        _lastTime = -1.0f;
        _duration = duration;
        _elapsed = 0.0f;
        _type = type;
        _function = function;
        _finishedFunctions = {finishedFunctions...};
    }

    bool update(float time);
    bool active() const { return _elapsed < _duration; }
};

#endif // TRANSITION_H
