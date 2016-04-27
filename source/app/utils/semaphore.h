#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class semaphore
{
private:
    std::mutex _mutex;
    std::condition_variable _condition;
    unsigned int _count;

public:
    explicit semaphore(int count = 0);

    void notify();
    void wait();
};

#endif // SEMAPHORE_H
