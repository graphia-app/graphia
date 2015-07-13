#ifndef MOVABLEPOINTER_H
#define MOVABLEPOINTER_H

#include "cpp1x_hacks.h"
#include <memory>

// Helper template that allows you to maintain a vector of
// Ts without the hassle of writing a move constructor for T
// (The sacrifice being cache locality, of course)
template<typename T> class MovablePointer : private std::unique_ptr<T>
{
public:
    template <typename... Args> MovablePointer(Args... args) :
        std::unique_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...))
    {}

    explicit MovablePointer(MovablePointer&& other) noexcept : std::unique_ptr<T>(std::move(other)) {}

    inline operator T*() const { return this->get(); }
};

#endif // MOVABLEPOINTER_H
