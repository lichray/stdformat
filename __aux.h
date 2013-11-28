/*-
 * Copyright (c) 2013 Zhihao Yuan.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _STDEX___AUX_H
#define _STDEX___AUX_H

#include "traits_adaptors.h"

#include <limits>

namespace stdex {

namespace detail {

inline namespace aux {

template <int N>
struct or_shift
{
	template <typename Int>
	static constexpr
	auto apply(Int n) -> Int
	{
		return or_shift<N / 2>::apply(n | (n >> N));
	}
};

template <>
struct or_shift<1>
{
	template <typename Int>
	static constexpr
	auto apply(Int n) -> Int
	{
		return n | (n >> 1);
	}
};

template <typename Int, typename R = typename std::make_unsigned<Int>::type>
constexpr
auto pow2_roundup(Int n) -> R
{
	return or_shift
	    <
		std::numeric_limits<R>::digits / 2
	    >
	    ::apply(R(n) - 1) + 1;
}

template <typename T, typename V>
using not_void_or_t = If_t<std::is_void<T>, identity_of<V>, identity_of<T>>;

template <typename From, typename To, typename = void>
struct is_nonarrow_convertible_impl : std::is_convertible<From, To> {};

template <>
struct is_nonarrow_convertible_impl<long double, double> : std::false_type {};

template <>
struct is_nonarrow_convertible_impl<long double, float> : std::false_type {};

template <>
struct is_nonarrow_convertible_impl<double, float> : std::false_type {};

template <typename T, typename = void>
struct is_unscoped_enum : std::false_type {};

template <typename T>
struct is_unscoped_enum<T, If_t<std::is_enum<T>>> :
	composed
	<
	    curried<std::is_convertible>::call<T>::template call,
	    std::underlying_type
	>
	::template call<T>
{};

template <typename From, typename To>
struct is_nonarrow_convertible_impl<From, To,
    If_t
    <
	and_also
	<
	    std::is_floating_point<From>,
	    std::is_integral<To>
	>
    >> : std::false_type
{};

template <typename From, typename To>
struct is_nonarrow_convertible_impl<From, To,
    If_t
    <
	and_also
	<
	    either<std::is_integral, std::is_enum>::call<From>,
	    std::is_floating_point<To>
	>
    >> : std::false_type
{};

template <typename T, typename V>
struct is_representable : bool_constant
	<
	    (std::numeric_limits<T>::min() >= std::numeric_limits<V>::min() and
	     std::numeric_limits<T>::max() <= std::numeric_limits<V>::max())
	>
{};

template <typename From, typename To>
struct is_nonarrow_convertible_impl<From, To,
    If_t
    <
	and_also
	<
	    std::is_integral<From>,
	    std::is_integral<To>
	>
    >> : is_representable<From, To>
{};

template <typename From, typename To>
struct is_nonarrow_convertible_impl<From, To,
    If_t
    <
	and_also
	<
	    is_unscoped_enum<From>,
	    std::is_integral<To>
	>
    >> : composed
	<
	    curried<is_representable>::call,
	    std::underlying_type
	>
	::apply<From, To>
{};

template <typename From, typename To>
struct is_nonarrow_convertible : is_nonarrow_convertible_impl
	<
	    typename std::remove_cv<From>::type,
	    typename std::remove_cv<To>::type
	>
{};

}

}

}

#endif
