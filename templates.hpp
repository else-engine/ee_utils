/**
 * Copyright (c) 2017 Gauthier ARNOULD
 * This file is released under the zlib License (Zlib).
 * See file LICENSE or go to https://opensource.org/licenses/Zlib
 * for full license details.
 */

#pragma once

#include <type_traits>

namespace ee {
namespace tutil {

/**
 * Just an alias to enable_if_t.
 * Templates meta programming is really verbose...
 */
template <bool B, typename T = void>
using eif = std::enable_if_t<B, T>;

/**
 * Split input signature R (C::*)(Args...) to a class C and a function signature
 * R (Args...) with C cv-qualified same as input signature and R (Args...)
 * unqualified and finally instantiate template template parameter from them.
 * The idea is to help implementation of templates like has_subscript_operator
 * by freeing it of writing specializations for unqualified / const / volatile
 * and const-volatile signatures.
 */
namespace detail {

template <template <typename, typename, typename> typename F, typename S>
struct split_member_func_sig;

template <template <typename, typename, typename = void> typename F, typename C, typename R, typename ... Args>
struct split_member_func_sig<F, R (C::*)(Args...)> {
    static constexpr bool value = F<C, R (Args...)>::value;
};

template <template <typename, typename, typename = void> typename F, typename C, typename R, typename ... Args>
struct split_member_func_sig<F, R (C::*)(Args...) const> {
    static constexpr bool value = F<const C, R (Args...)>::value;
};

template <template <typename, typename, typename = void> typename F, typename C, typename R, typename ... Args>
struct split_member_func_sig<F, R (C::*)(Args...) volatile> {
    static constexpr bool value = F<volatile C, R (Args...)>::value;
};

template <template <typename, typename, typename = void> typename F, typename C, typename R, typename ... Args>
struct split_member_func_sig<F, R (C::*)(Args...) const volatile> {
    static constexpr bool value = F<const volatile C, R (Args...)>::value;
};

} // namespace detail

/**
 * Test if a class has a subscript operator with a given signature.
 */
namespace detail {

template <typename C, typename S, typename = void>
struct has_subscript_operator_impl : std::false_type {};

template <typename C, typename R, typename... Args>
struct has_subscript_operator_impl<C, R (Args...), eif<
std::is_same<decltype(std::declval<C>().operator[](std::declval<Args>()...)), R>::value
>> : std::true_type {};

} // namespace detail

template <typename S>
constexpr bool has_subscript_operator = detail::split_member_func_sig<detail::has_subscript_operator_impl, S>::value;

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
} // namespace ee
