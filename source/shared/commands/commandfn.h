#ifndef COMMANDFN_H
#define COMMANDFN_H

#include <functional>

class Command;
class CommandFn
{
public:
    template<typename Fn, typename ReturnType>
    using EnableIfFnReturnTypeIs = typename std::enable_if<
        std::is_same<
            typename std::result_of<Fn(Command&)>::type,
            ReturnType
        >::value
    >::type;

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, bool>>
    CommandFn(Fn&& fn)
    {
        _boolFn = fn;
    }

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, void>, typename = void>
    CommandFn(Fn&& fn)
    {
        _voidFn = fn;
    }

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
