#ifndef COMMANDFN_H
#define COMMANDFN_H

#include <functional>

class Command;
class CommandFn
{
public:
    template<typename Fn, typename ReturnType>
    using EnableIfFnReturnTypeIs = typename std::enable_if_t<
        std::is_same<
            typename std::result_of_t<Fn(Command&)>,
            ReturnType
        >::value &&
        // Avoid enabling the universal move constructors if Fn is a CommandFn
        !std::is_same<std::remove_reference_t<Fn>, CommandFn>::value
    >;

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, bool>>
    CommandFn(Fn&& fn) : _boolFn(fn) {}

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, void>, typename = void>
    CommandFn(Fn&& fn) : _voidFn(fn) {}

    CommandFn() = default;
    CommandFn(const CommandFn& other) = default;
    CommandFn(CommandFn&& other) = default;
    CommandFn& operator=(const CommandFn& other) = default;
    CommandFn& operator=(CommandFn&& other) = default;

    bool operator()(Command& command) const
    {
        if(_boolFn != nullptr)
            return _boolFn(command);

        _voidFn(command);
        return true;
    }

private:
    std::function<bool(Command&)> _boolFn;
    std::function<void(Command&)> _voidFn;
};

#endif // COMMANDFN_H
