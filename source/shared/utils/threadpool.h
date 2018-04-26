#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "singleton.h"
#include "function_traits.h"
#include "is_std_container.h"

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
#include <utility>
#include <type_traits>

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
    explicit ThreadPool(const QString& threadNamePrefix = QStringLiteral("Worker"),
               int numThreads = std::thread::hardware_concurrency());
    virtual ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

    bool saturated() const { return _activeThreads >= static_cast<int>(_threads.size()); }
    bool idle() const { return _activeThreads == 0; }

    template<typename Fn, typename... Args> using ReturnType = typename std::result_of_t<Fn(Args...)>;

    template<typename Fn, typename... Args> std::future<ReturnType<Fn, Args...>> execute(Fn f, Args&&... args)
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
    // If the concurrent function returns a value, give the ResultsType class a std::vector _values
    // member, which contains the results from each thread
    template<typename ResultsVectorOrVoid, bool = std::is_void<ResultsVectorOrVoid>::value>
    class ResultMember;

    template<typename ResultsVectorOrVoid>
    class ResultMember<ResultsVectorOrVoid, true> {};

    template<typename ResultsVectorOrVoid>
    class ResultMember<ResultsVectorOrVoid, false>
    {
    protected:
        mutable std::vector<ResultsVectorOrVoid> _values;
    };

    template<typename ResultsVectorOrVoid> class ResultsType : public ResultMember<ResultsVectorOrVoid>
    {
        friend class ThreadPool;

    private:
        mutable std::vector<std::future<ResultsVectorOrVoid>> _futures;

        explicit ResultsType(std::vector<std::future<ResultsVectorOrVoid>>&& futures) :
            _futures(std::move(futures))
        {}

    public:
        // When the concurrent function returns a value, move it out of the future into the
        // _values vector, provided by inheriting from the ResultsMember specialisation
        template<typename T = ResultsVectorOrVoid>
        typename std::enable_if_t<!std::is_void<T>::value, void>
        wait() const
        {
            for(auto& future : _futures)
            {
                future.wait();
                this->_values.emplace_back(std::move(future.get()));
            }
        }

        // When the concurrent function doesn't return a value, just wait for the results
        template<typename T = ResultsVectorOrVoid>
        typename std::enable_if_t<std::is_void<T>::value, void>
        wait() const
        {
            for(auto& future : _futures)
                future.wait();
        }

        // This iterator allows the results to be iterated over in a single pass
        class iterator
        {
        private:
            template<typename T, bool = is_std_sequence_container<typename T::value_type>::value>
            struct impl {};

            // If the concurrent function's return type is a sequence container, collapse its
            // values down, so that from the iterator user's point of view, they work with a single
            // homogenous sequence, rather than a vector of sequences
            template<typename T>
            struct impl<T, true>
            {
                typename std::vector<T>::iterator _threadIt;
                typename std::vector<T>::iterator _threadEndIt;
                typename T::iterator _resultsIt;
                typename T::value_type::iterator _containerIt;

                using value_type = typename T::value_type::value_type;

                void increment()
                {
                    if(_containerIt != _resultsIt->end())
                        ++_containerIt;

                    while(_containerIt == _resultsIt->end())
                    {
                        if(_resultsIt != _threadIt->end())
                            ++_resultsIt;

                        if(_resultsIt != _threadIt->end())
                            _containerIt = _resultsIt->begin();
                        else do
                        {
                            if(_threadIt != _threadEndIt)
                                ++_threadIt;

                            if(_threadIt == _threadEndIt)
                                break;

                            _resultsIt = _threadIt->begin();
                            _containerIt = _resultsIt->begin();

                        } while(_resultsIt == _threadIt->end());

                        if(_threadIt == _threadEndIt)
                        {
                            _resultsIt = {};
                            _containerIt = {};
                            break;
                        }
                    }
                }

                impl(std::vector<T>& values, bool end)
                {
                    if(!end)
                    {
                        _threadIt = values.begin();
                        _threadEndIt = values.end();

                        if(_threadIt != _threadEndIt)
                        {
                            _resultsIt = _threadIt->begin();
                            while(_threadIt != _threadEndIt && _resultsIt == _threadIt->end())
                                increment();

                            if(_threadIt != _threadEndIt && _resultsIt != _threadIt->end())
                            {
                                _containerIt = _resultsIt->begin();
                                while(_threadIt != _threadEndIt && _containerIt == _resultsIt->end())
                                    increment();
                            }
                        }

                        if(_threadIt == _threadEndIt)
                        {
                            _resultsIt = {};
                            _containerIt = {};
                        }
                    }
                    else
                    {
                        _threadIt = _threadEndIt = values.end();
                        _resultsIt = {};
                        _containerIt = {};
                    }
                }

                auto& valueReference() const
                {
                    Q_ASSERT(_containerIt != _resultsIt->end());
                    return *_containerIt;
                }

                bool operator==(const impl& other) const
                {
                    if(_threadIt != other._threadIt)
                        return false;

                    if(_resultsIt != other._resultsIt)
                        return false;

                    return _containerIt == other._containerIt;
                }
            };

            template<typename T>
            struct impl<T, false>
            {
                typename std::vector<T>::iterator _threadIt;
                typename std::vector<T>::iterator _threadEndIt;
                typename T::iterator _resultsIt;

                using value_type = typename T::value_type;

                void increment()
                {
                    if(_resultsIt != _threadIt->end())
                        ++_resultsIt;

                    while(_resultsIt == _threadIt->end())
                    {
                        if(_threadIt != _threadEndIt)
                            ++_threadIt;

                        if(_threadIt == _threadEndIt)
                            break;

                        _resultsIt = _threadIt->begin();
                    }

                    if(_threadIt == _threadEndIt)
                        _resultsIt = {};
                }

                impl(std::vector<T>& values, bool end)
                {
                    if(!end)
                    {
                        _threadIt = values.begin();
                        _threadEndIt = values.end();

                        if(_threadIt != _threadEndIt)
                        {
                            _resultsIt = _threadIt->begin();
                            while(_threadIt != _threadEndIt && _resultsIt == _threadIt->end())
                                increment();
                        }

                        if(_threadIt == _threadEndIt)
                            _resultsIt = {};
                    }
                    else
                    {
                        _threadIt = _threadEndIt = values.end();
                        _resultsIt = {};
                    }
                }

                auto& valueReference() const
                {
                    Q_ASSERT(_resultsIt != _threadIt->end());
                    return *_resultsIt;
                }

                bool operator==(const impl& other) const
                {
                    if(_threadIt != other._threadIt)
                        return false;

                    return _resultsIt == other._resultsIt;
                }
            };

            impl<ResultsVectorOrVoid> _impl;

        public:
            using self_type = iterator;
            using value_type = typename impl<ResultsVectorOrVoid>::value_type;
            using reference = value_type&;
            using pointer = value_type*;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = size_t;

            iterator(ResultsType* results, bool end) :
                _impl(results->_values, end)
            {}

            self_type operator++()
            {
                self_type i = *this;
                _impl.increment();
                return i;
            }

            auto& operator*() const
            {
                return _impl.valueReference();
            }

            bool operator!=(const self_type& other) const { return !operator==(other); }
            bool operator==(const self_type& other) const
            {
                return _impl == other._impl;
            }
        };

        template<typename T = ResultsVectorOrVoid>
        typename std::enable_if_t<!std::is_void<T>::value, iterator>
        begin() { return iterator(this, false); }

        template<typename T = ResultsVectorOrVoid>
        typename std::enable_if_t<!std::is_void<T>::value, iterator>
        end() { return iterator(this, true); }
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
        using ResultsVectorOrVoid = std::vector<Result>;

        ResultsVectorOrVoid operator()(It it, It last, Fn& f)
        {
            ResultsVectorOrVoid values;
            values.reserve(std::distance(it, last));

            for(; it != last; ++it)
                values.emplace_back(std::move(this->invoke(f, it)));

            return values;
        }
    };

    template<typename It, typename Fn>
    struct Executor<It, Fn, void> : Invoker<It, Fn>
    {
        using ResultsVectorOrVoid = void;

        ResultsVectorOrVoid operator()(It it, It last, Fn& f) const
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
            _first(std::move(first)), _last(std::move(last))
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
        ResultsType<typename FnExecutor<It, Fn>::ResultsVectorOrVoid>;

    template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn f, bool blocking = true)
    {
        Coster<It> coster(first, last);

        const auto totalCost = coster.total();
        const auto numThreads = static_cast<int>(_threads.size());
        const auto costPerThread = totalCost / numThreads +
                ((totalCost % numThreads) ? 1 : 0);

        static_assert(std::is_convertible<ArgumentType<Fn>, It>::value ||
                      std::is_convertible<ArgumentType<Fn>, typename It::value_type>::value,
                      "Fn's argument must be an It or an It::value_type");

        FnExecutor<It, Fn> executor;
        std::vector<std::future<typename FnExecutor<It, Fn>::ResultsVectorOrVoid>> futures;

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

            futures.emplace_back(execute([executor, it, threadLast, f]() mutable
            {
                return executor(it, threadLast, f);
            }));

            it = threadLast;
        }

        auto results = Results<It, Fn>(std::move(futures));

        if(blocking)
            results.wait();

        return results;
    }
};

class ThreadPoolSingleton : public ThreadPool, public Singleton<ThreadPoolSingleton> {};

template<typename Fn, typename... Args> auto execute_on_threadpool(Fn&& f, Args&&... args)
{
    return S(ThreadPoolSingleton)->execute(std::forward<Fn>(f), args...);
}

template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn&& f, bool blocking = true)
{
    return S(ThreadPoolSingleton)->concurrent_for(first, last, std::forward<Fn>(f), blocking);
}

#endif // THREADPOOL_H
