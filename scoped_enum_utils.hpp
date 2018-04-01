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

namespace detail {

template <typename, typename B>
struct is_scoped_enum_impl : std::false_type {};

template <typename E>
struct is_scoped_enum_impl<E, std::true_type> :
std::bool_constant< ! std::is_convertible<E, std::underlying_type_t<E>>::value>::type {};

} // namespace detail

template <typename E>
constexpr bool is_scoped_enum =
detail::is_scoped_enum_impl<E, typename std::is_enum<E>::type>::value;

/**
 * Easy convert scoped enum value to integral.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, std::underlying_type_t<E>> as_value(E element) {
    return static_cast<std::underlying_type_t<E>>(element);
}

/**
 * Easy convert integral value to scoped enum value.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> as(std::underlying_type_t<E> value) {
    return static_cast<E>(value);
}

/**
 * Bitwise left shift operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator<<(E lhs, std::size_t rhs) {
    return static_cast<E>(as_value(lhs) << rhs);
}

/**
 * Bitwise left shift assignment operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E&> operator<<=(E& lhs, std::size_t rhs) {
    lhs = lhs << rhs;

    return lhs;
}

/**
 * Bitwise right shift operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator>>(E lhs, std::size_t rhs) {
    return static_cast<E>(as_value(lhs) >> rhs);
}

/**
 * Bitwise right shift assignment operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E&> operator>>=(E& lhs, std::size_t rhs) {
    lhs = lhs >> rhs;

    return lhs;
}

/**
 * Bitwise AND operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator|(E lhs, E rhs) {
    return static_cast<E>(as_value(lhs) | as_value(rhs));
}

/**
 * Bitwise AND assignment operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E&> operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;

    return lhs;
}

/**
 * Bitwise OR operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator&(E lhs, E rhs) {
    return static_cast<E>(as_value(lhs) & as_value(rhs));
}

/**
 * Bitwise OR assignment operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E&> operator&=(E& lhs, E rhs) {
    lhs = lhs & rhs;

    return lhs;
}

/**
 * Bitwise XOR operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator^(E lhs, E rhs) {
    return static_cast<E>(as_value(lhs) ^ as_value(rhs));
}

/**
 * Bitwise XOR assignment operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E&> operator^=(E& lhs, E rhs) {
    lhs = lhs ^ rhs;

    return lhs;
}

/**
 * Bitwise complement operator for scoped enum used as bitfield.
 */
template <typename E>
inline constexpr eif<is_scoped_enum<E>, E> operator~(E x) {
    return static_cast<E>( ~ as_value(x));
}

/**
 * Returns whether all the bits set in y are set in x.
 */
template <typename T>
inline constexpr bool all(T x, T y) {
    return (x & y) == y;
}

/**
 * Returns whether any of the bits set in y is set in x.
 */
template <typename T>
inline constexpr bool any(T x, T y) {
    return (x & y) != T{0};
}

/**
 * Returns whether none of the bits set in y are set in x.
 */
template <typename T>
inline constexpr bool none(T x, T y) {
    return ! any(x, y);
}

/**
 * Allow iterating over all enum elements.
 * Consecutive elements must have consecutive numeric values.
 */
template <typename E, E F, E L, typename = eif<is_scoped_enum<E>>>
struct Range {
    static_assert(as_value(F) <= as_value(L));

    static constexpr E first = F;
    static constexpr E last  = L;
    static constexpr auto first_value = as_value(F);
    static constexpr auto last_value  = as_value(L);
    static constexpr auto count = last_value - first_value + 1;

    static constexpr auto index_from(E e) {
        return as_value(e) - first_value;
    }

    static constexpr auto enum_from(std::underlying_type_t<E> i) {
        return static_cast<E>(i + first_value);
    }

    static constexpr bool has(E e) {
        auto e_value = as_value(e);

        return first_value <= e_value && e_value <= last_value;
    }

    class Iterator {
        public:
            constexpr Iterator(E element) :
                value_{as_value(element)} {}

            constexpr E operator*() const {
                return static_cast<E>(value_);
            }

            constexpr Iterator& operator++() {
                ++ value_;

                return *this;
            }

            constexpr bool operator!=(Iterator rhs) const {
                return value_ != rhs.value_;
            }

        private:
            std::underlying_type_t<E> value_;
    };
};

template <typename E, E F, E L, typename = eif<is_scoped_enum<E>>>
constexpr auto begin(Range<E, F, L>) {
    constexpr typename Range<E, F, L>::Iterator it = F;
    return it;
}

template <typename E, E F, E L, typename = eif<is_scoped_enum<E>>>
constexpr auto end(Range<E, F, L>) {
    constexpr auto it = ++ typename Range<E, F, L>::Iterator{L};
    return it;
}

} // namespace ee