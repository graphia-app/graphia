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
#include <lexy/grammar.hpp>
#include <lexy/lexeme.hpp>

#include <QByteArray>
#include <QString>

#include <cstddef>
#include <functional>

namespace lexyutils
{
namespace dsl = lexy::dsl;

// The grammars are all byte oriented, i.e. they use lexy::default_encoding, so lexy's
// Unicode aware character classes can't be applied to them wholesale; in particular
// the complement operator is only available for classes that are known not to be
// Unicode, which is only the case for those built from dsl::lit_b. These are the byte
// oriented equivalents, from which "any byte except x" is expressed as anyByte - x

// Matches any single byte; the lit_b makes the class a non-Unicode one, so that it
// can be complemented, and the byte it excludes is then unioned back in
inline constexpr auto anyByte = -dsl::lit_b<0x80> / dsl::lit_b<0x80>;

// Matches any single non-ASCII byte, i.e. anything in the range 0x80 to 0xFF
inline constexpr auto nonAsciiByte = anyByte - dsl::ascii::character;

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

template<typename Lexeme>
QString qStringFrom(const Lexeme& lexeme)
{
    return QString::fromUtf8(lexeme.data(), static_cast<qsizetype>(lexeme.size()));
}

template<typename Lexeme>
double doubleFrom(const Lexeme& lexeme)
{
    return byteArrayFrom(lexeme).toDouble();
}

inline constexpr auto asQString = lexy::callback<QString>(
    [](const auto& lexeme) { return qStringFrom(lexeme); });

inline constexpr auto asDouble = lexy::callback<double>(
    [](const auto& lexeme) { return doubleFrom(lexeme); });

template<typename Value>
inline constexpr auto asNumber = lexy::callback<Value>([](const auto& lexeme) -> Value
{
    const auto bytes = byteArrayFrom(lexeme);

    bool isInt = false;
    const auto integer = bytes.toInt(&isInt);

    if(isInt)
        return integer;

    return bytes.toDouble();
});

struct QuotedString : lexy::token_production
{
    static constexpr auto rule = dsl::lit_c<'"'> >>
        (dsl::capture(dsl::token(dsl::while_(
            LEXY_LIT("\\\"") | (anyByte - dsl::lit_c<'"'>)))) + dsl::lit_c<'"'>);

    static constexpr auto value = lexy::callback<QString>([](const auto& lexeme)
    {
        auto text = qStringFrom(lexeme);
        text.replace("\\\"", "\"");
        return text;
    });
};

class progress_input
{
public:
    using encoding = lexy::default_encoding;
    using char_type = encoding::char_type;

private:
    static constexpr size_t POLL_INTERVAL = 4096;

    struct State
    {
        const char_type* _begin = nullptr;
        const char_type* _end = nullptr;

        size_t _poll = POLL_INTERVAL;

        std::function<void(size_t position)> _onPositionChangedFn;
        std::function<bool()> _cancelledFn;

        void poll(size_t position)
        {
            _poll = position + POLL_INTERVAL;

            if(_onPositionChangedFn != nullptr)
                _onPositionChangedFn(position);

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

            const auto position = static_cast<size_t>(_cur - _state->_begin);
            if(position >= _state->_poll)
                _state->poll(position);
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
