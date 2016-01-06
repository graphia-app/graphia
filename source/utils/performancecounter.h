#ifndef PERFORMANCECOUNTER_H
#define PERFORMANCECOUNTER_H

#include <chrono>
#include <deque>
#include <functional>

class PerformanceCounter
{
public:
    using ReportFn = std::function<void(float)>;
    explicit PerformanceCounter(std::chrono::seconds interval);

    void setReportFn(ReportFn f) { _f = f; }

    void tick();
    float ticksPerSecond();

private:
    static const int MAX_SAMPLES = 16;
    std::deque<std::chrono::time_point<std::chrono::high_resolution_clock>> _samples;

    std::chrono::time_point<std::chrono::high_resolution_clock> _lastReport;
    std::chrono::seconds _interval;
    ReportFn _f = [](float) {};
};

#endif // PERFORMANCECOUNTER_H
