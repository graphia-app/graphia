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
    auto elapsed = _elapsedTimer.elapsed();
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
            auto sum = std::accumulate(samples.begin(), samples.end(), 0);
            double mean = static_cast<double>(sum) / samples.size();
            auto minMax = std::minmax_element(samples.begin(), samples.end());

            double stdDev = 0.0;
            for(const auto& value : samples)
                stdDev += ((value - mean) * (value - mean));

            stdDev /= samples.size();
            stdDev = std::sqrt(stdDev);

            qDebug() << name << QString("%1/%2/%3/%4 ms (mean/min/max/stddev)")
                .arg(mean).arg(*minMax.first).arg(*minMax.second).arg(stdDev);
        }
        else
            qDebug() << name << samples.front() << "ms";
    }
}
