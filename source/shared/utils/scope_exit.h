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

#ifndef SCOPE_EXIT_H
#define SCOPE_EXIT_H

#include <utility>

// This is more or less directly lifted from N4189

// modeled slightly after Andrescu’s talk and article(s)
namespace std::experimental{ // NOLINT cert-dcl58-cpp
template <typename EF>
struct scope_exit {
    // construction
    explicit
    scope_exit(EF &&f) noexcept
        :exit_function(std::move(f))
        ,execute_on_destruction{true}{}
    // move
    scope_exit(scope_exit &&rhs) noexcept
        :exit_function(std::move(rhs.exit_function))
        ,execute_on_destruction{rhs.execute_on_destruction}{
        rhs.release();
    }
    // release
    ~scope_exit() noexcept/*(noexcept(this->exit_function()))*/{
        if (execute_on_destruction)
            this->exit_function();
    }
    void release() noexcept { this->execute_on_destruction=false;}

    scope_exit(scope_exit const &)=delete;
    void operator=(scope_exit const &)=delete;
    scope_exit& operator=(scope_exit &&)=delete;

private:
    EF exit_function;
    bool execute_on_destruction = false; // exposition only
};
template <typename EF>
auto make_scope_exit(EF &&exit_function) noexcept {
    return scope_exit<std::remove_reference_t<EF>>(std::forward<EF>(exit_function));
}
} // namespace std::experimental

#endif // SCOPE_EXIT_H
