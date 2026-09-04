/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef LEXY_UTILS_H
#define LEXY_UTILS_H

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/encoding.hpp>
#include <lexy/lexeme.hpp>

#include <QByteArray>
#include <QString>

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

// Helpers shared by the various lexy based grammars
namespace lexyutils
{
namespace dsl = lexy::dsl;

namespace detail
{
template<unsigned char... Offsets>
constexpr auto makeNonAsciiByte(std::integer_sequence<unsigned char, Offsets...>)
{
    return (dsl::lit_b<static_cast<unsigned char>(0x80u + Offsets)> / ...);
}
} // namespace detail

// The grammars are all byte oriented, i.e. they use lexy::default_encoding, which
// means that lexy's Unicode aware character classes (and in particular the
// complement operator) can't be used with them; these are the byte oriented
// equivalents, from which "any byte except x" is expressed as anyByte - x

// Matches any single non-ASCII byte, i.e. anything in the range 0x80 to 0xFF
inline constexpr auto nonAsciiByte =
    detail::makeNonAsciiByte(std::make_integer_sequence<unsigned char, 128>{});

// Matches any single byte
inline constexpr auto anyByte = dsl::ascii::character / nonAsciiByte;

inline constexpr auto sign = dsl::lit_c<'+'> / dsl::lit_c<'-'>;
inline constexpr auto exponent =
    (dsl::lit_c<'e'> / dsl::lit_c<'E'>) >> (dsl::if_(sign) + dsl::digits<>);

// Matches a number, whether integral or real
inline constexpr auto number = dsl::token(
    dsl::if_(sign) +
    ((dsl::lit_c<'.'> >> dsl::digits<>) |
    (dsl::digits<> >> dsl::if_(dsl::lit_c<'.'> >> dsl::if_(dsl::digits<>)))) +
    dsl::if_(exponent));

// Matches only strictly real numbers, i.e. those that have
// a decimal point and/or an exponent; 1 is not a real, 1.0 is
inline constexpr auto real = dsl::token(
    dsl::if_(sign) +
    ((dsl::lit_c<'.'> >> (dsl::digits<> + dsl::if_(exponent))) |
    (dsl::digits<> >> ((dsl::lit_c<'.'> >> (dsl::if_(dsl::digits<>) + dsl::if_(exponent))) |
    exponent))));

template<typename Lexeme>
QByteArray byteArrayFrom(const Lexeme& lexeme)
{
    return {lexeme.data(), static_cast<qsizetype>(lexeme.size())};
}

// Any lexeme the grammars produce is a sequence of (presumed to be) UTF-8 encoded bytes
template<typename Lexeme>
QString qStringFrom(const Lexeme& lexeme)
{
    return QString::fromUtf8(lexeme.data(), static_cast<qsizetype>(lexeme.size()));
}

// Yields a value only if the lexeme is an integer that's representable as an int
template<typename Lexeme>
std::optional<int> intFrom(const Lexeme& lexeme)
{
    bool success = false;
    const auto value = byteArrayFrom(lexeme).toInt(&success);

    if(!success)
        return std::nullopt;

    return value;
}

template<typename Lexeme>
double doubleFrom(const Lexeme& lexeme)
{
    return byteArrayFrom(lexeme).toDouble();
}

// lexy callback that converts a captured lexeme into a QString
inline constexpr auto asQString = lexy::callback<QString>(
    [](const auto& lexeme) { return qStringFrom(lexeme); });

// lexy callback that converts a captured real lexeme into a double
inline constexpr auto asDouble = lexy::callback<double>(
    [](const auto& lexeme) { return doubleFrom(lexeme); });

// A lexy Input over a contiguous block of bytes that periodically reports how far
// through it the parser has progressed, and which can be made to appear as if it
// has reached EOF, thereby terminating any parse that is in progress
class progress_input
{
public:
    using encoding = lexy::default_encoding;
    using char_type = encoding::char_type;

private:
    // Number of bytes consumed between successive polls; the parser bumps its
    // position one byte at a time, and calling out on every one of these is
    // needlessly expensive, given that neither the progress indication nor the
    // response to cancellation need be especially fine grained
    static constexpr size_t POLL_INTERVAL = 4096;

    struct State
    {
        const char_type* _begin = nullptr;
        const char_type* _end = nullptr;
        const char_type* _poll = nullptr;

        std::function<void(size_t position)> _onPositionChangedFn;
        std::function<bool()> _cancelledFn;

        void poll(const char_type* position)
        {
            _poll = position + POLL_INTERVAL;

            if(_onPositionChangedFn != nullptr)
                _onPositionChangedFn(static_cast<size_t>(position - _begin));

            if(_cancelledFn != nullptr && _cancelledFn())
            {
                // Move the end before the current position,
                // effectively making everything look like EOF
                _end = _begin;
            }
        }
    };

    mutable State _state;

public:
    class Reader
    {
    public:
        using encoding = progress_input::encoding;
        using iterator = const char_type*;

        struct marker
        {
            iterator _it;

            iterator position() const noexcept { return _it; }
        };

        Reader(State* state, iterator cur) noexcept :
            _state(state), _cur(cur)
        {}

        encoding::int_type peek() const noexcept
        {
            if(_cur >= _state->_end)
                return encoding::eof();

            return encoding::to_int_type(*_cur);
        }

        void bump() noexcept
        {
            _cur++;

            if(_cur >= _state->_poll)
                _state->poll(_cur);
        }

        iterator position() const noexcept { return _cur; }

        marker current() const noexcept { return {_cur}; }
        void reset(marker m) noexcept { _cur = m._it; }

    private:
        State* _state = nullptr;
        iterator _cur = nullptr;
    };

    progress_input(const char_type* data, size_t size)
    {
        _state._begin = data;
        _state._end = data + size;
        _state._poll = data + POLL_INTERVAL;
    }

    template<typename OnPositionChangedFn>
    void onPositionChanged(const OnPositionChangedFn& onPositionChangedFn)
    {
        _state._onPositionChangedFn = onPositionChangedFn;
    }

    template<typename CancelledFn>
    void setCancelledFn(const CancelledFn& cancelledFn)
    {
        _state._cancelledFn = cancelledFn;
    }

    Reader reader() const& noexcept { return {&_state, _state._begin}; }
};

} // namespace lexyutils

#endif // LEXY_UTILS_H
