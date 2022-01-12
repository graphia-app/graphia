/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef MOVABLEPOINTER_H
#define MOVABLEPOINTER_H

#include <memory>

// Helper template that allows you to maintain a vector of
// Ts without the hassle of writing a move constructor for T
// (The sacrifice being cache locality, of course)
template<typename T> class MovablePointer : private std::unique_ptr<T>
{
public:
    template <typename... Args> explicit MovablePointer(Args... args) :
        std::unique_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...))
    {}

    MovablePointer(MovablePointer&& other) noexcept : std::unique_ptr<T>(std::move(other)) {}

    MovablePointer(const MovablePointer& other) = delete;
    void operator=(const MovablePointer& other) = delete;

    inline operator T*() const { return this->get(); } //NOLINT
};

#endif // MOVABLEPOINTER_H
