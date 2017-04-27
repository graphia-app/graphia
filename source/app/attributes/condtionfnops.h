#ifndef CONDTIONFNOPS_H
#define CONDTIONFNOPS_H

namespace ConditionFnOp
{
    enum class Logical
    {
        Or,
        And
    };

    enum class Equality
    {
        Equal,
        NotEqual
    };

    enum class Numerical
    {
        LessThan,
        GreaterThan,
        LessThanOrEqual,
        GreaterThanOrEqual
    };

    enum class String
    {
        Includes,
        Excludes,
        Starts,
        Ends,
        MatchesRegex
    };

    enum class Unary
    {
        HasValue
    };
}

#endif // CONDTIONFNOPS_H
