/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
        explicit _Then(QFutureWatcher<AsyncFnResult>* watcher) :
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
