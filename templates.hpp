/**
 * Copyright (c) 2017 Gauthier ARNOULD
 * This file is released under the zlib License (Zlib).
 * See file LICENSE or go to https://opensource.org/licenses/Zlib
 * for full license details.
 */

#pragma once

#include <type_traits>

namespace EE {
namespace tutil {

/**
 * Just an alias to enable_if_t.
 * Templates meta programming is really verbose...
 */
template <bool B, typename T = void>
using eif = std::enable_if_t<B, T>;

/**
 * is_arithmetic as in "can it be used for arithmetic?", so include reference to
 * arithmetic types.
 */
template <typename T>
constexpr bool is_arithmetic = std::is_arithmetic<std::decay_t<T>>::value;

/**
 * all_same
 */
namespace detail {

template <typename...>
struct all_same : std::true_type {};

template <typename T>
struct all_same<T> : std::true_type {};

template <typename F, typename S, typename... Rs>
struct all_same<F, S, Rs...>
    : std::integral_constant<bool, std::is_same<F, S>::value && all_same<S, Rs...>::value> {};

} // namespace detail

template <typename... Ts>
constexpr bool all_same = detail::all_same<Ts...>::value;

} // namespace tutil
} // namespace EE
