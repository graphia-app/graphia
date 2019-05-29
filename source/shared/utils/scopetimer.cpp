#include "scopetimer.h"

#include "shared/utils/container.h"

#include <QDebug>

#include <algorithm>
#include <cmath>

ScopeTimer::ScopeTimer(QString name, size_t numSamples) :
    _name(std::move(name)), _numSamples(numSamples)
{
    _elapsedTimer.start();
}

ScopeTimer::~ScopeTimer()
{
    if(_elapsedTimer.isValid())
        stop();
}

void ScopeTimer::stop()
{
    auto elapsed = _elapsedTimer.nsecsElapsed();
    ScopeTimerManager::instance()->submit(_name, elapsed, _numSamples);
    _elapsedTimer.invalidate();
}

void ScopeTimerManager::submit(const QString& name, qint64 elapsed, size_t numSamples)
{
    std::unique_lock<std::mutex> lock(_mutex);

    auto& result = _results[name];

    while(result.size() >= numSamples)
        result.pop_front();

    result.push_back(elapsed);
}

void ScopeTimerManager::reportToQDebug() const
{
    std::unique_lock<std::mutex> lock(_mutex);

    for(const auto& result : _results)
    {
        const auto& name = result.first;
        const auto& samples = result.second;

        if(samples.size() > 1)
        {
            auto sum = std::accumulate(samples.begin(), samples.end(), 0LL);
            double mean = static_cast<double>(sum) / samples.size();
            auto minMax = std::minmax_element(samples.begin(), samples.end());

            double stdDev = 0.0;
            for(const auto& value : samples)
                stdDev += ((value - mean) * (value - mean));

            stdDev /= samples.size();
            stdDev = std::sqrt(stdDev);

            mean /= 1000000.0;
            auto min = static_cast<double>(*minMax.first) / 1000000.0;
            auto max = static_cast<double>(*minMax.second) / 1000000.0;
            stdDev /= 1000000.0;

            qDebug() << name << QStringLiteral("%1/%2/%3/%4 ms (mean/min/max/stddev)")
                .arg(mean).arg(min).arg(max).arg(stdDev);
        }
        else
            qDebug() << name << (samples.front() / 1000000.0) << "ms";
    }
}
