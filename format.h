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

#ifndef _STDEX_FORMAT_H
#define _STDEX_FORMAT_H

#include "__formatter.h"

#include <tuple>
#include <functional>

namespace stdex {

namespace detail {

template <typename CharT, typename Traits, typename Allocator, typename Tuple>
inline
auto vformat(Allocator const& a, basic_string_view<CharT> fmt, Tuple tp)
	-> std::basic_string<CharT, Traits, Allocator>
{
	std::basic_string<CharT, Traits, Allocator> buf(a);

	return buf;
}

template <typename T, typename V>
using not_void_or_t = If_t<std::is_void<T>, identity_of<V>, identity_of<T>>;

}

template
<
    typename Traits = void,
    typename Allocator,
    typename... T
>
inline
auto format(Allocator const& a,
            basic_string_view<typename Allocator::value_type> fmt,
            T const&... t)
	-> std::basic_string
	<
	    typename Allocator::value_type,
	    detail::not_void_or_t
	    <Traits, std::char_traits<typename Allocator::value_type>>,
	    Allocator
	>
{
	return detail::vformat
		<
		    typename Allocator::value_type,
		    detail::not_void_or_t
		    <Traits, std::char_traits<typename Allocator::value_type>>
		>
		(a, fmt, std::forward_as_tuple(t...));
}

template <typename Traits, typename... T>
inline
auto format(basic_string_view<typename Traits::char_type> fmt, T const&... t)
	-> std::basic_string
	<
	    typename Traits::char_type,
	    Traits,
	    std::allocator<typename Traits::char_type>
	>
{
	return format<Traits>(std::allocator<typename Traits::char_type>(),
	    fmt, t...);
}

template <typename... T>
inline
std::string format(string_view fmt, T const&...t)
{
	return format<std::string::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::wstring format(wstring_view fmt, T const&...t)
{
	return format<std::wstring::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::u16string format(u16string_view fmt, T const&...t)
{
	return format<std::u16string::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::u32string format(u32string_view fmt, T const&...t)
{
	return format<std::u32string::traits_type>(fmt, t...);
}

}

#endif