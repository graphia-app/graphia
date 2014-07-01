#ifndef UNIQUE_LOCK_WITH_SIDE_EFFECTS_H
#define UNIQUE_LOCK_WITH_SIDE_EFFECTS_H

#include <mutex>
#include <functional>

// Call a lambda after a lock is released
// It may be that this is a terrible idea
template<typename T> struct unique_lock_with_side_effects
{
public:
    unique_lock_with_side_effects(T& mutex) :
        _lock(mutex) {}
    unique_lock_with_side_effects(T& mutex, std::function<void()> f) :
        _lock(mutex), _f(f) {}
    ~unique_lock_with_side_effects()
    {
        if(_lock.owns_lock())
            _lock.unlock();

        if(_f)
            _f();
    }

    unique_lock_with_side_effects(unique_lock_with_side_effects&& other) :
        _lock(std::move(other._lock)), _f(std::move(other._f))
    {}

    void setPostUnlockAction(std::function<void()> f) { _f = f; }

private:
    std::unique_lock<T> _lock;
    std::function<void()> _f;
};

#endif // UNIQUE_LOCK_WITH_SIDE_EFFECTS_H
