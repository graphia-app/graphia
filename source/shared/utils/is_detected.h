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

#ifndef IS_DETECTED_H
#define IS_DETECTED_H

#include <type_traits>

namespace std::experimental{ // NOLINT cert-dcl58-cpp

// Copy and pasted from https://en.cppreference.com/w/cpp/experimental/is_detected
namespace detail {
template <class Default, class AlwaysVoid,
          template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  // Note that std::void_t is a C++17 feature
  using value_t = std::true_type;
  using type = Op<Args...>;
};

} // namespace detail

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template< class Default, template<class...> class Op, class... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;

} // namespace std::experimental
#endif // IS_DETECTED_H
