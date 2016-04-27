#include "performancecounter.h"

PerformanceCounter::PerformanceCounter(std::chrono::seconds interval) :
    _interval(interval)
{}

void PerformanceCounter::tick()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> now =
            std::chrono::high_resolution_clock::now();

    _samples.emplace_back(now);

    while(_samples.size() > MAX_SAMPLES)
        _samples.pop_front();

    if(now > _lastReport + _interval)
    {
        _f(ticksPerSecond());
        _lastReport = now;
    }
}

float PerformanceCounter::ticksPerSecond()
{
    float mean = 0.0f;
    size_t numSamples = _samples.size();

    for(size_t index = 0; index < numSamples - 1; index++)
    {
        auto start = _samples.at(index);
        auto end = _samples.at(index + 1);

        std::chrono::duration<float> duration = end - start;

        mean += duration.count() / numSamples;
    }

    if(mean == 0.0f)
        return 0.0f;

    return 1.0f / mean;
}
