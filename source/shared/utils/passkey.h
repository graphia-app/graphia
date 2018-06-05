#ifndef PASSKEY_H
#define PASSKEY_H

// https://stackoverflow.com/a/3218920/2721809

template<typename T>
class PassKey
{
    friend T;
    PassKey() {} // NOLINT
    PassKey(PassKey const&) {} // NOLINT
};

#endif // PASSKEY_H
