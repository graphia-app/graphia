#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>

#include <functional>

class Transition : public QObject
{
    Q_OBJECT
public:
    enum class Type
    {
        None,
        Linear,
        EaseInEaseOut,
        Power,
        InversePower
    };

private:
    Type _type;
    float _lastTime;
    float _duration;
    float _elapsed;
    std::function<void(float)> _function;
    std::vector<std::function<void()>> _finishedFunctions;

public:
    Transition();

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
    bool finished() const { return _elapsed >= _duration; }
};

#endif // TRANSITION_H
