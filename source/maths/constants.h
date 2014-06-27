#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef _MSC_VER
// Sigh...
#define constexpr const
#endif

class Constants
{
public:
    static constexpr float Pi() { return std::atan2(0, -1); }
    static constexpr float TwoPi() { return 2.0f * Pi(); }
};

#endif // CONSTANTS_H
