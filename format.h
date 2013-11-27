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

template <typename CharT, size_t N>
inline
auto swiden(char const (&s)[N])
	-> std::basic_string<CharT>
{
	return { s, s + N - 1 };
}

template <typename CharT>
inline
int parse_int(basic_string_view<CharT>& s)
{
	int n = 0;
	auto it = s.begin();

	for (; it != s.end() and ('0' <= *it and *it <= '9'); ++it)
	{
		n *= 10;
		n += *it - '0';
	}

	s.remove_prefix(it - s.begin());

	return n;
}

template <int Low, int High, int Mid = (Low + High) / 2, typename = void>
struct write_arg_at_impl;

template <int Low, int High, int Mid>
struct write_arg_at_impl<Low, High, Mid, If_ct<(Low > High)>>
{
	template <typename Tuple, typename Writer>
	static void apply(int n, Tuple tp, Writer w)
	{
		throw std::out_of_range
		{
		    "tuple index out of range"
		};
	}
};

template <int Mid>
struct write_arg_at_impl<Mid, Mid, Mid, void>
{
	template
	<
	    typename Tuple, typename Writer,
	    typename T = typename std::decay
	    <
		typename std::tuple_element<Mid - 1, Tuple>::type
	    >
	    ::type
	>
	static void apply(int n, Tuple tp, Writer w)
	{
		if (n != Mid)
			throw std::out_of_range
			{
			    "tuple index out of range"
			};

		formatter<T>().output(w, std::get<Mid - 1>(tp));
	}
};

template <int Low, int High, int Mid>
struct write_arg_at_impl<Low, High, Mid, If_ct<(Low < High)>>
{
	template <typename Tuple, typename Writer>
	static void apply(int n, Tuple tp, Writer w)
	{
		if (n < Mid)
			write_arg_at_impl<Low, Mid - 1>::apply(n, tp, w);
		else if (n == Mid)
			write_arg_at_impl<Mid, Mid>::apply(n, tp, w);
		else
			write_arg_at_impl<Mid + 1, High>::apply(n, tp, w);
	}
};

template <typename Tuple, typename Writer>
inline
void write_arg_at(int n, Tuple tp, Writer w)
{
	write_arg_at_impl<1, std::tuple_size<Tuple>{}>::apply(n, tp, w);
}

template <typename CharT, typename Traits, typename Allocator, typename Tuple>
inline
auto vformat(Allocator const& a, basic_string_view<CharT> fmt, Tuple tp)
	-> std::basic_string<CharT, Traits, Allocator>
{
	using spec_type = basic_string_view<CharT>;
	using string_type = std::basic_string<CharT, Traits, Allocator>;
	using writer_type = format_writer<string_type>;

	static const auto curly_braces = swiden<CharT>("{}");

	string_type buf(a);
	int arg_index = 0;
	bool sequential;

	while (1)
	{
		auto off = fmt.find_first_of(curly_braces);

		if (off == spec_type::npos)
		{
			buf.append(fmt.data(), fmt.size());
			break;
		}
		else
		{
			buf.append(fmt.data(), off);
		}

		auto ch = fmt[off];
		fmt.remove_prefix(off + 1);

		if (ch == '}')
		{
			if (fmt.empty() or fmt.front() != '}')
				throw std::invalid_argument
				{
				    "Single '}' encountered in format string"
				};

			buf.push_back('}');
			fmt.remove_prefix(1);
			continue;
		}

		if (fmt.empty())
			throw std::invalid_argument
			{
			    "Single '{' encountered in format string"
			};

		if (fmt.front() == '{')
		{
			buf.push_back('{');
			fmt.remove_prefix(1);
			continue;
		}

		ch = fmt.front();

		if ('0' < ch && ch <= '9')
		{
			if (arg_index == 0)
				sequential = false;

			else if (sequential)
				throw std::invalid_argument
				{
				    "cannot switch from automatic field "
				    "numbering to manual field specification"
				};

			arg_index = parse_int(fmt);
		}
		else
		{
			if (arg_index == 0)
				sequential = true;

			else if (not sequential)
				throw std::invalid_argument
				{
				    "cannot switch from manual field "
				    "specification to automatic field numbering"
				};

			++arg_index;
		}

		if (fmt.empty())
			throw std::invalid_argument
			{
			    "unmatched '{' in format"
			};

		ch = fmt.front();
		fmt.remove_prefix(1);

		if (ch == '}')
		{
			write_arg_at(arg_index, tp, writer_type(buf));
		}
	}

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
