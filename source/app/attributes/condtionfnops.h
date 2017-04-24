#ifndef CONDTIONFNOPS_H
#define CONDTIONFNOPS_H

namespace ConditionFnOp
{
    enum class Binary
    {
        Or,
        And
    };

    enum class Numerical
    {
        Equal,
        NotEqual,
        LessThan,
        GreaterThan,
        LessThanOrEqual,
        GreaterThanOrEqual
    };

    enum class String
    {
        Equal,
        NotEqual,
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
