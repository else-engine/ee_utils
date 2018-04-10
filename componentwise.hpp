/**
 * Copyright (c) 2017-2018 Gauthier ARNOULD
 * This file is released under the zlib License (Zlib).
 * See file LICENSE or go to https://opensource.org/licenses/Zlib
 * for full license details.
 */

#pragma once

#include <type_traits>

#include "templates.hpp"

namespace ee {

using tutil::eif;

/**
 * Allow generating a container type of the same size but with another value
 * type. It has to be specialized for any type we want to use with it.
 */
template <typename T, typename VT>
struct but;

template <typename T, typename VT>
using but_t = typename but<T, VT>::type;

/**
 * ee::but specialization for std::array.
 */
template <typename VT, typename T, std::size_t S>
struct but<std::array<T, S>, VT> {
    using type = std::array<VT, S>;
};

/**
 * is_tuple. in the mathematical meaning.
 * To be considered as tuple a class needs :
 *  - a constexpr static member "size" reporting the number of components.
 *  - a type member "value_type", the type of the components.
 *  - a subscript operator which returns value_type references from 0 to size-1.
 *  - TODO to be an aggregate (needs std::is_aggregate)
 */
namespace detail {

template <typename T, typename = void>
struct is_tuple_impl : std::false_type {};

template <typename T>
struct is_tuple_impl<T, eif<
tutil::has_subscript_operator<typename T::reference (T::*)(std::size_t)> &&
tutil::has_subscript_operator<typename T::const_reference (T::*)(std::size_t) const> &&
(std::tuple_size<T>::value * sizeof(typename T::value_type) == sizeof(T) || std::tuple_size<T>::value == 0)
>> : std::true_type {};

} // namespace detail

template <typename T>
constexpr bool is_tuple = detail::is_tuple_impl<std::decay_t<T>>::value;

/**
 * Find the value type contained in a type. For basic types like int or float,
 * simply returns it as is, for tuple types like std::array or math::vec,
 * returns type of the contained elements.
 */
namespace detail {

template <typename T, typename = void>
struct find_value_type_impl {
    using type = T;
};

template <typename T>
struct find_value_type_impl<T, eif<is_tuple<T>>> {
    using type = typename T::value_type;
};

} // namespace detail

template <typename T>
using find_value_type = typename detail::find_value_type_impl<T>::type;

/**
 * Find the first in a list of types to be a tuple. Set type to void if no any
 * tuple is present in the list.
 */
namespace detail {

template <bool, typename... Ts>
struct find_first_tuple_impl {
    using type = void;
};

template <typename F, typename S, typename... Ts>
struct find_first_tuple_impl<false, F, S, Ts...> : public find_first_tuple_impl<is_tuple<S>, S, Ts...> {};

template <typename F, typename... Ts>
struct find_first_tuple_impl<true, F, Ts...> {
    using type = std::decay_t<F>;
};

} // namespace detail

template <typename... Ts>
using find_first_tuple = typename detail::find_first_tuple_impl<false, void, Ts...>::type;

/**
 * Allow calling functions componentwise.
 */
namespace detail {

template <typename Fn, typename... Ts>
using cwise_return =
but_t<find_first_tuple<Ts...>, std::decay_t<std::invoke_result_t<Fn, find_value_type<std::decay_t<Ts>>...>>>;

/* 1 parameter */
// tuple
template <typename Fn, typename T1, std::size_t... Is>
constexpr eif<is_tuple<T1>, cwise_return<Fn, T1>> cwise(Fn&& fn, T1&& t1, std::index_sequence<Is...>) {
    return {std::invoke(fn, t1[Is])...};
}

/* 2 parameters */
// tuple, tuple
template <typename Fn, typename T1, typename T2, std::size_t... Is>
constexpr eif<is_tuple<T1> && is_tuple<T2>, cwise_return<Fn, T1, T2>> cwise(Fn&& fn, T1&& t1, T2&& t2, std::index_sequence<Is...>) {
    return {std::invoke(fn, t1[Is], t2[Is])...};
}

// tuple, scalar
template <typename Fn, typename T1, typename S2, std::size_t... Is>
constexpr eif<is_tuple<T1> && ! is_tuple<S2>, cwise_return<Fn, T1, S2>> cwise(Fn&& fn, T1&& t1, S2&& s2, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, t1[Is], s2)...};
}

// scalar, tuple
template <typename Fn, typename S1, typename T2, std::size_t... Is>
constexpr eif< ! is_tuple<S1> && is_tuple<T2>, cwise_return<Fn, S1, T2>> cwise(Fn&& fn, S1&& s1, T2&& t2, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, s1, t2[Is])...};
}

/* 3 parameters */
// tuple, tuple, tuple
template <typename Fn, typename T1, typename T2, typename T3, std::size_t... Is>
constexpr eif<is_tuple<T1> && is_tuple<T2> && is_tuple<T3>, cwise_return<Fn, T1, T2, T3>> cwise(Fn&& fn, T1&& t1, T2&& t2, T3&& t3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, t1[Is], t2[Is], t3[Is])...};
}

// tuple, tuple, scalar
template <typename Fn, typename T1, typename T2, typename S3, std::size_t... Is>
constexpr eif<is_tuple<T1> && is_tuple<T2> && ! is_tuple<S3>, cwise_return<Fn, T1, T2, S3>> cwise(Fn&& fn, T1&& t1, T2&& t2, S3&& s3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, t1[Is], t2[Is], s3)...};
}

// tuple, scalar, tuple
template <typename Fn, typename T1, typename S2, typename T3, std::size_t... Is>
constexpr eif<is_tuple<T1> && ! is_tuple<S2> && is_tuple<T3>, cwise_return<Fn, T1, S2, T3>> cwise(Fn&& fn, T1&& t1, S2&& s2, T3&& t3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, t1[Is], s2, t3[Is])...};
}

// scalar, tuple, tuple
template <typename Fn, typename S1, typename T2, typename T3, std::size_t... Is>
constexpr eif< ! is_tuple<S1> && is_tuple<T2> && is_tuple<T3>, cwise_return<Fn, S1, T2, T3>> cwise(Fn&& fn, S1&& s1, T2&& t2, T3&& t3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, s1, t2[Is], t3[Is])...};
}

// tuple, scalar, scalar
template <typename Fn, typename T1, typename S2, typename S3, std::size_t... Is>
constexpr eif<is_tuple<T1> && ! is_tuple<S2> && ! is_tuple<S3>, cwise_return<Fn, T1, S2, S3>> cwise(Fn&& fn, T1&& t1, S2&& s2, S3&& s3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, t1[Is], s2, s3)...};
}

// scalar, tuple, scalar
template <typename Fn, typename S1, typename T2, typename S3, std::size_t... Is>
constexpr eif< ! is_tuple<S1> && is_tuple<T2> && ! is_tuple<S3>, cwise_return<Fn, S1, T2, S3>> cwise(Fn&& fn, S1&& s1, T2&& t2, S3&& s3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, s1, t2[Is], s3)...};
}

// scalar, scalar, tuple
template <typename Fn, typename S1, typename S2, typename T3, std::size_t... Is>
constexpr eif< ! is_tuple<S1> && ! is_tuple<S2> && is_tuple<T3>, cwise_return<Fn, S1, S2, T3>> cwise(Fn&& fn, S1&& s1, S2&& s2, T3&& t3, std::index_sequence<Is...>) {
    return {std::invoke<Fn>(fn, s1, s2, t3[Is])...};
}

} // namespace detail

// cwise main entry point
template <typename Fn, typename... Ts>
constexpr decltype(auto) cwise(Fn&& fn, Ts&&... ts) {
    using tuple = find_first_tuple<Ts...>;

    static_assert( ! std::is_void_v<tuple>, "cwise needs at least one argument to be tuple type");
    static_assert(sizeof...(Ts) > 0 && sizeof...(Ts) < 4, "cwise accepts 1 to 3 data arguments");

    return detail::cwise(std::forward<Fn>(fn), std::forward<Ts>(ts)..., std::make_index_sequence<std::tuple_size_v<tuple>>());
}

/**
 * Use tuple elements as distinct parameters with any callable.
 */
namespace detail {

template <typename Fn, typename T, std::size_t... Is>
constexpr decltype(auto) split(Fn&& fn, T&& t, std::index_sequence<Is...>) {
    return std::invoke<Fn>(std::forward<Fn>(fn), t[Is]...);
}

} // namespace detail

template <typename Fn, typename T, typename = eif<is_tuple<T>>>
constexpr decltype(auto) split(Fn&& fn, T&& t) {
    return detail::split(std::forward<Fn>(fn), std::forward<T>(t),
                         std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

} // namespace ee
