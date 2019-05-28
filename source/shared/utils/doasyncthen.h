#ifndef DOASYNCTHEN_H
#define DOASYNCTHEN_H

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <type_traits>

namespace u
{
    template<typename AsyncFnResult>
    class _Then
    {
    private:
        QFutureWatcher<AsyncFnResult>* _watcher = nullptr;
        bool _deleteWatcherOnDestruction = true;

    public:
        _Then(QFutureWatcher<AsyncFnResult>* watcher) :
            _watcher(watcher)
        {}

        ~_Then()
        {
            if(_deleteWatcherOnDestruction)
            {
                // .then was never called
                delete _watcher;
            }
        }

        template<typename ThenFn>
        void then(ThenFn&& thenFn)
        {
            _deleteWatcherOnDestruction = false;

            QObject::connect(_watcher, &QFutureWatcher<AsyncFnResult>::finished,
            [thenFn, watcher = _watcher]
            {
                thenFn(watcher->result());
                watcher->deleteLater();
            });
        }
    };

    template<typename AsyncFn>
    auto doAsync(AsyncFn&& thisFn)
    {
        using AsyncFnResult = std::invoke_result_t<AsyncFn>;

        auto future = QtConcurrent::run(thisFn);
        auto* watcher = new QFutureWatcher<AsyncFnResult>;
        watcher->setFuture(future);

        return _Then<AsyncFnResult>(watcher);
    }
} // namespace u

#endif // DOASYNCTHEN_H
