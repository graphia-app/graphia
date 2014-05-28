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
    float lastTime;
    float _duration;
    float elapsed;
    std::function<void(float)> _function;
    std::function<void()> _finishedFunction;

public:
    Transition();

    void start(float duration, Type type, std::function<void(float)> function,
               std::function<void()> finishedFunction = [](){});

    bool update(float time);
    bool finished() const { return elapsed >= _duration; }
};

#endif // TRANSITION_H
