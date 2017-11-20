#ifndef CANCELLABLE_H
#define CANCELLABLE_H

#include <atomic>

class Cancellable
{
private:
    std::atomic_bool _cancelled;

public:
    Cancellable() : _cancelled(false) {}
    virtual ~Cancellable() = default;

    virtual void uncancel() { _cancelled = false; }
    virtual void cancel() { _cancelled = true; }
    virtual bool cancelled() const { return _cancelled; }
};

#endif // CANCELLABLE_H
