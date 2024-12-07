/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

#include <type_traits>

namespace u
{
    template<typename AsyncFnResult>
    class _Then
    {
    private:
        QFuture<AsyncFnResult> _future;

    public:
        explicit _Then(QFuture<AsyncFnResult>&& future) :
            _future(std::move(future))
        {}

        // NOLINTNEXTLINE cppcoreguidelines-missing-std-forward
        template<typename ThenFn> void then(ThenFn&& thenFn)
        {
            auto* watcher = new QFutureWatcher<AsyncFnResult>;
            QObject::connect(watcher, &QFutureWatcher<AsyncFnResult>::finished,
            [thenFn = std::forward<ThenFn>(thenFn), watcher]
            {
                if constexpr(!std::is_same_v<AsyncFnResult, void>)
                    thenFn(watcher->result());
                else
                    thenFn();

                watcher->deleteLater();
            });

            watcher->setFuture(_future);
        }
    };

    template<typename AsyncFn>
    auto doAsync(AsyncFn&& thisFn)
    {
        auto future = QtConcurrent::run(std::forward<AsyncFn>(thisFn));

        return _Then<std::invoke_result_t<AsyncFn>>(std::move(future));
    }
} // namespace u

#endif // DOASYNCTHEN_H
