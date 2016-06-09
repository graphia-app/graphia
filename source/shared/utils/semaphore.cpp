#include "semaphore.h"

semaphore::semaphore(int count) :
      _count(count)
{}

void semaphore::notify()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _count++;
    _condition.notify_one();
}

void semaphore::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return _count > 0; });
    _count--;
}
