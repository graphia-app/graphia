/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef VOID_CALLABLE_WRAPPER_H
#define VOID_CALLABLE_WRAPPER_H

#include <memory>

class void_callable_wrapper
{
private:
    struct callable_base
    {
        virtual ~callable_base() = default;
        virtual void call() = 0;
    };

    template<typename F>
    struct callable : callable_base
    {
        F _f;
        explicit callable(F f) : _f(std::move(f)) {}

        void call() override { _f(); }
    };

    std::unique_ptr<callable_base> _callable;

public:
    template<typename F>
    explicit void_callable_wrapper(F f) : _callable(std::make_unique<callable<F>>(std::move(f))) {}

    void_callable_wrapper() = delete;
    void_callable_wrapper(const void_callable_wrapper&) = delete;
    void_callable_wrapper(void_callable_wrapper&&) = default;
    void_callable_wrapper& operator=(const void_callable_wrapper&) = delete;
    void_callable_wrapper& operator=(void_callable_wrapper&&) = default;

    void operator()() { _callable->call(); }
};

#endif // VOID_CALLABLE_WRAPPER_H
