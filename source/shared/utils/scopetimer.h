#ifndef SCOPE_TIMER_H
#define SCOPE_TIMER_H

#include "shared/utils/singleton.h"

#include <map>
#include <deque>
#include <mutex>

#include <QString>
#include <QElapsedTimer>

// Include this header and insert SCOPE_TIMER into your code OR
// use SCOPE_TIMER_MULTISAMPLES(<numSamples>) OR
// manually create a ScopeTimer timer(<uniqueName>);

class ScopeTimer
{
public:
    explicit ScopeTimer(QString name, size_t numSamples = 1);
    ~ScopeTimer();

    void stop();

private:
    QString _name;
    size_t _numSamples;
    QElapsedTimer _elapsedTimer;
};

#if defined(__GNUC__) || defined(__clang__)
#define SCOPE_TIMER_FUNCTION static_cast<const char*>(__PRETTY_FUNCTION__)
#else
#define SCOPE_TIMER_FUNCTION static_cast<const char*>(__func__)
#endif

#ifdef BUILD_SOURCE_DIR
#define SCOPE_TIMER_FILENAME QString(__FILE__).replace(BUILD_SOURCE_DIR, "")
#else
#define SCOPE_TIMER_FILENAME static_cast<const char*>(__FILE__)
#endif

#define SCOPE_TIMER_CONCAT2(a, b) a ## b
#define SCOPE_TIMER_CONCAT(a, b) SCOPE_TIMER_CONCAT2(a, b)
#define SCOPE_TIMER_INSTANCE_NAME SCOPE_TIMER_CONCAT(_scopeTimer, __COUNTER__)
#define SCOPE_TIMER_FILE_LINE __FILE__ ## ":" ## __LINE__
#define SCOPE_TIMER_MULTISAMPLES(samples) \
    ScopeTimer SCOPE_TIMER_INSTANCE_NAME( \
        QString("%1:%2 %3").arg(SCOPE_TIMER_FILENAME).arg(__LINE__).arg(SCOPE_TIMER_FUNCTION), samples);
#define SCOPE_TIMER SCOPE_TIMER_MULTISAMPLES(1)

class ScopeTimerManager : public Singleton<ScopeTimerManager>
{
public:
    void submit(const QString& name, qint64 elapsed, size_t numSamples);
    void reportToQDebug() const;

private:
    mutable std::mutex _mutex;
    std::map<QString, std::deque<qint64>> _results;
};

#endif // SCOPE_TIMER_H
