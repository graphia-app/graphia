#ifndef STATIC_VISITOR_H
#define STATIC_VISITOR_H

namespace u
{
    // This is a template that multiply inherits from its parameters,
    // and exposes the operator() of each, the net result being the
    // ability to dispatch different bits of code depending on the type
    // of the thing passed to operator(). e.g.:
    //
    // u::static_visitor
    // {
    //     [](TypeA thing) { /* do a TypeA thing */ },
    //     [](TypeB thing) { /* do a TypeB thing */ }
    // } v;
    //
    // TypeA a;
    // TypeB b;
    //
    // v(a);
    // v(b);

    template<typename... Base>
    struct static_visitor : Base...
    {
        using Base::operator()...;
    };

    // Deduction guide, so we can invoke the static_visitor constructor without
    // having to specify various complicated template parameters
    template<typename... T> static_visitor(T...) -> static_visitor<T...>;
} // namespace u

#endif // STATIC_VISITOR_H
