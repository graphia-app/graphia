#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "singleton.h"
#include "function_traits.h"

#include <QString>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template<typename It> static It incrementIterator(It it, It last, const int n)
{
    return it + std::min(n, static_cast<const int>(std::distance(it, last)));
}

class ThreadPool
{
private:
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _waitForNewTask;
    std::queue<std::function<void()>> _tasks;
    std::atomic<bool> _stop;
    std::atomic<int> _activeThreads;

public:
    ThreadPool(const QString& threadNamePrefix = QStringLiteral("Worker"),
               int numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    bool saturated() const { return _activeThreads >= static_cast<int>(_threads.size()); }
    bool idle() const { return _activeThreads == 0; }

    template<typename Fn, typename... Args> using ReturnType = typename std::result_of_t<Fn(Args...)>;

    template<typename Fn, typename... Args> std::future<ReturnType<Fn, Args...>> execute_on_threadpool(Fn f, Args&&... args)
    {
        if(_stop)
            return std::future<ReturnType<Fn, Args...>>();

        auto taskPtr = std::make_shared<std::packaged_task<ReturnType<Fn, Args...>(Args...)>>(f);

        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks.push(
                [taskPtr, args...]() mutable
                {
                    (*taskPtr)(std::forward<Args>(args)...);
                });
        }

        // Wake a thread up
        _waitForNewTask.notify_one();
        _activeThreads++;
        return taskPtr->get_future();
    }

private:
    template<typename ValueType> class ResultsType
    {
        friend class ThreadPool;

    private:
        std::vector<std::future<ValueType>> _futures;

        explicit ResultsType(std::vector<std::future<ValueType>>& futures) :
            _futures(std::move(futures))
        {}

    public:
        ResultsType(ResultsType&& other) :
            _futures(std::move(other._futures))
        {}

        void wait() const
        {
            for(auto& future : _futures)
                future.wait();
        }

        template<typename T = ValueType> typename std::enable_if_t<!std::is_void<T>::value, T>
        get()
        {
            ValueType values;

            for(auto& future : _futures)
            {
                auto v = std::move(future.get());
                values.reserve(values.size() + v.size());
                values.insert(values.end(), std::make_move_iterator(v.begin()),
                                            std::make_move_iterator(v.end()));
            }

            return values;
        }
    };

    template<typename Fn> using ArgumentType = typename function_traits<Fn>::template arg<0>::type;

    // Fn may take a value/reference or an iterator; this template determines how to call it
    template<typename It, typename Fn> struct Invoker
    {
        using ReturnType = std::result_of_t<Fn(ArgumentType<Fn>)>;

        // Fn argument is an iterator
        template<typename T = ReturnType>
        static typename std::enable_if_t<std::is_convertible<ArgumentType<Fn>, It>::value, T>
        invoke(Fn& f, It& it) { return f(it); }

        // Fn argument is an value/reference
        template<typename T = ReturnType>
        static typename std::enable_if_t<std::is_convertible<ArgumentType<Fn>, typename It::value_type>::value, T>
        invoke(Fn& f, It it) { return f(*it); }
    };

    template<typename It, typename Fn, typename Result>
    struct Executor : Invoker<It, Fn>
    {
        using ValueType = std::vector<Result>;

        ValueType operator()(It it, It last, Fn& f)
        {
            ValueType values;

            for(; it != last; ++it)
                values.emplace_back(this->invoke(f, it));

            return values;
        }
    };

    template<typename It, typename Fn>
    struct Executor<It, Fn, void> : Invoker<It, Fn>
    {
        using ValueType = void;

        ValueType operator()(It it, It last, Fn& f) const
        {
            for(; it != last; ++it)
                this->invoke(f, it);
        }
    };

    template<typename It, typename Fn> using FnExecutor =
        Executor<It, Fn, typename std::result_of_t<Fn(ArgumentType<Fn>)>>;

    template<typename It>
    class CosterBase
    {
    protected:
        It _first;
        It _last;

    public:
        CosterBase(It first, It last) :
            _first(first), _last(last)
        {}
    };

    // When It::value_type::computeCostHint() doesn't exist, we get
    // this implementation, which gives every element equal weight
    template<typename It, typename Enable = void>
    struct Coster : public CosterBase<It>
    {
        using CosterBase<It>::CosterBase;

        int total() { return std::distance(this->_first, this->_last); }
        int operator()(It) { return 1; }
    };

    // When It::value_type::computeCostHint() does exist, we get this
    // implementation, that allows elements to hint how much computation
    // it will cost and balance the thread/work allocation accordingly
    template<typename It>
    struct Coster<It,
        std::enable_if_t<std::is_member_function_pointer<
            decltype(&It::value_type::computeCostHint)>::value>> :
        public CosterBase<It>
    {
        using CosterBase<It>::CosterBase;

        int total()
        {
            int n = 0;

            for(auto it = this->_first; it != this->_last; ++it)
                n += it->computeCostHint();

            return n;
        }

        int operator()(It it) { return it->computeCostHint(); }
    };

public:
    template<typename It, typename Fn> using Results =
        ResultsType<typename FnExecutor<It, Fn>::ValueType>;

    template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn f, bool blocking = true)
    {
        Coster<It> coster(first, last);

        const int totalCost = coster.total();
        const int numThreads = static_cast<int>(_threads.size());
        const int costPerThread = totalCost / numThreads +
                ((totalCost % numThreads) ? 1 : 0);

        static_assert(std::is_convertible<ArgumentType<Fn>, It>::value ||
                      std::is_convertible<ArgumentType<Fn>, typename It::value_type>::value,
                      "Fn's argument must be an It or an It::value_type");

        FnExecutor<It, Fn> executor;
        std::vector<std::future<typename FnExecutor<It, Fn>::ValueType>> futures;

        for(It it = first; it != last;)
        {
            It threadLast = it;
            int cost = 0;
            do
            {
                cost += coster(threadLast);
                ++threadLast;
            }
            while(threadLast != last && cost < costPerThread);

            futures.emplace_back(execute_on_threadpool([executor, it, threadLast, f]() mutable
            {
                return executor(it, threadLast, f);
            }));

            it = threadLast;
        }

        auto results = Results<It, Fn>(futures);

        if(blocking)
            results.wait();

        return results;
    }
};

class ThreadPoolSingleton : public ThreadPool, public Singleton<ThreadPoolSingleton> {};

template<typename Fn, typename... Args> auto execute_on_threadpool(Fn&& f, Args&&... args)
{
    return S(ThreadPoolSingleton)->execute_on_threadpool(std::move(f), args...);
}

template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn&& f, bool blocking = true)
{
    return S(ThreadPoolSingleton)->concurrent_for(first, last, std::move(f), blocking);
}

#endif // THREADPOOL_H
