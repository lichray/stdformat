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

}

}

}

#endif
