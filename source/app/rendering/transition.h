/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>

#include <functional>
#include <limits>

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
    Type _type = Type::None;
    float _duration = 0.0f;
    float _elapsed = std::numeric_limits<float>::max();
    std::function<void(float)> _function = [](float) {};
    std::vector<std::function<void()>> _finishedFunctions;
    bool _finishing = false;
    bool _suppressSignals = false;

public:
    template<typename Fn>
    Transition& start(float duration, Type type, Fn&& function)
    {
        Q_ASSERT(!_finishing);

        if(!active() && !_suppressSignals)
            emit started();

        _duration = duration;
        _elapsed = 0.0f;
        _type = type;
        _function = std::forward<Fn>(function);
        _finishedFunctions.clear();
        _suppressSignals = false;

        return *this;
    }

    template<typename FinishedFn>
    Transition& then(FinishedFn&& finishedFn)
    {
        _finishedFunctions.emplace_back(std::forward<FinishedFn>(finishedFn));
        return *this;
    }

    template<typename FinishedFn>
    Transition& alternativeThen(FinishedFn&& finishedFn)
    {
        if(!_finishedFunctions.empty())
            _finishedFunctions.pop_back();

        return then(std::forward<FinishedFn>(finishedFn));
    }

    bool update(float dTime);
    bool active() const { return _elapsed == 0.0f || _elapsed < _duration; }
    void cancel();

    // The idea behind this is that when you have a chain of transitions, you
    // don't necessarily want to signal when each starts and finishes, so by
    // calling this from a finishedFunction, the finished signal of the current
    // transition is not emitted, nor is the started signal of the subsequent
    // transition, the net result being the entire chain of transitions has a
    // single started and single finished signal emitted
    void willBeImmediatelyReused();

signals:
    void started();
    void finished();
};

#endif // TRANSITION_H
