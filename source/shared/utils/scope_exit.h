#ifndef SCOPE_EXIT_H
#define SCOPE_EXIT_H

// This is more or less directly lifted from N4189

// modeled slightly after Andrescu’s talk and article(s)
namespace std{ // NOLINT
namespace experimental{
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
    bool execute_on_destruction; // exposition only
};
template <typename EF>
auto make_scope_exit(EF &&exit_function) noexcept {
    return scope_exit<std::remove_reference_t<EF>>(std::forward<EF>(exit_function));
}
} // namespace experimental
} // namespace std

#endif // SCOPE_EXIT_H
