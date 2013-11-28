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

template <typename CharT>
inline
bool leads_digits(CharT ch)
{
	return '0' < ch and ch <= '9';
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

enum class adjustment
{
	unspecified,
	left,
	right,
};

template <int Low, int High, int Mid = (Low + High) / 2, typename = void>
struct write_arg_at_impl;

template <int Low, int High, int Mid>
struct write_arg_at_impl<Low, High, Mid, If_ct<(Low > High)>>
{
	template <typename Tuple, typename Writer, typename... Opts>
	static
	void apply(int n, Tuple tp, Writer w, Opts... o)
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
	template <typename Tuple, typename Writer, typename... Opts>
	static
	void apply(int n, Tuple tp, Writer w, Opts... o)
	{
		using T = typename std::decay
		    <
			typename std::tuple_element<Mid - 1, Tuple>::type
		    >
		    ::type;

		if (n != Mid)
			throw std::out_of_range
			{
			    "tuple index out of range"
			};

		do_format<Mid - 1, T>(w, tp, o...);
	}

private:

	template <int I, typename T, typename Writer, typename Tuple>
	static
	void do_format(Writer w, Tuple tp)
	{
		formatter<T>().output(w, std::get<I>(tp));
	}

	template <int I, typename T, typename Writer, typename Tuple,
	          typename Spec>
	static
	void do_format_with_spec(Writer w, Tuple tp, Spec spec, std::true_type)
	{
		formatter<T>(spec).output(w, std::get<I>(tp));
	}

	template <int I, typename T, typename Writer, typename Tuple,
	          typename Spec>
	static
	void do_format_with_spec(Writer w, Tuple tp, Spec spec, std::false_type)
	{
		throw std::invalid_argument
		{
		    "Disabled format specifier"
		};
	}

	template <int I, typename T, typename Writer, typename Tuple>
	static
	void do_format(Writer w, Tuple tp, adjustment adj)
	{

		decide_justification<T>(w, adj, 0);
		do_format<I, T>(w, tp);
		w.justify_content();
	}

	template <int I, typename T, typename Writer, typename Tuple,
	          typename Spec>
	static
	void do_format(Writer w, Tuple tp, adjustment adj, Spec spec)
	{

		decide_justification<T>(w, adj, 0);
		do_format_with_spec<I, T>(w, tp, spec,
		    std::is_constructible<formatter<T>, Spec>());
		w.justify_content();
	}

	template <typename T, typename Writer>
	static
	void decide_justification(Writer& w, adjustment adj, ...)
	{
		if (adj != adjustment::left)
			w.padding_left();
	}

	template <typename T, typename Writer>
	static
	void decide_justification(Writer& w, adjustment adj,
	    typename formatter<T>::default_left_justified* = 0)
	{
		if (adj == adjustment::right)
			w.padding_left();
	}
};

template <int Low, int High, int Mid>
struct write_arg_at_impl<Low, High, Mid, If_ct<(Low < High)>>
{
	template <typename Tuple, typename Writer, typename... Opts>
	static
	void apply(int n, Tuple tp, Writer w, Opts... o)
	{
		if (n < Mid)
			write_arg_at_impl<Low, Mid - 1>::apply(n, tp, w, o...);
		else if (n == Mid)
			write_arg_at_impl<Mid, Mid>::apply(n, tp, w, o...);
		else
			write_arg_at_impl<Mid + 1, High>::apply(n, tp, w, o...);
	}
};

template <typename Tuple, typename Writer, typename... Opts>
inline
void write_arg_at(int n, Tuple tp, Writer w, Opts... o)
{
	write_arg_at_impl<1, std::tuple_size<Tuple>{}>::apply(n, tp, w, o...);
}

template <typename CharT, typename Traits, typename Allocator, typename Tuple>
auto vformat(Allocator const& a, basic_string_view<CharT> fmt, Tuple tp)
	-> std::basic_string<CharT, Traits, Allocator>
{
	using spec_type = basic_string_view<CharT>;
	using string_type = std::basic_string<CharT, Traits, Allocator>;
	using writer_type = format_writer<string_type>;

	auto expect_more = [&]()
	{
		if (fmt.empty())
			throw std::invalid_argument
			{
			    "unmatched '{' in format"
			};
	};

	string_type buf(a);
	int arg_index = 0;
	bool sequential;

	buf.reserve(pow2_roundup(fmt.size()));

	while (1)
	{
		auto off = fmt.find_first_of("{}");

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

		if (leads_digits(fmt.front()))
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

		expect_more();
		ch = fmt.front();
		fmt.remove_prefix(1);

		if (ch == ':')
		{
			adjustment adj = adjustment::unspecified;
			int width = 0;

			expect_more();

			switch (fmt.front())
			{
			case '<':
				adj = adjustment::left;
				break;
			case '>':
				adj = adjustment::right;
				break;
			}

			if (adj != adjustment::unspecified)
			{
				fmt.remove_prefix(1);
				expect_more();
			}

			if (leads_digits(fmt.front()))
				width = parse_int(fmt);

			auto off = fmt.find('}');

			if (off == spec_type::npos)
				throw std::invalid_argument
				{
				    "unmatched '{' in format"
				};

			else if (off == 0)
				write_arg_at(arg_index, tp,
				    writer_type(buf, width), adj);

			else
				write_arg_at(arg_index, tp,
				    writer_type(buf, width), adj,
				    fmt.substr(0, off));

			fmt.remove_prefix(off + 1);
		}

		else if (ch == '}')
		{
			write_arg_at(arg_index, tp, writer_type(buf));
		}

		else
		{
			throw std::invalid_argument
			{
			    "expecting ':' or '}'"
			};
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
std::string format(string_view fmt, T const&... t)
{
	return format<std::string::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::wstring format(wstring_view fmt, T const&... t)
{
	return format<std::wstring::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::u16string format(u16string_view fmt, T const&... t)
{
	return format<std::u16string::traits_type>(fmt, t...);
}

template <typename... T>
inline
std::u32string format(u32string_view fmt, T const&... t)
{
	return format<std::u32string::traits_type>(fmt, t...);
}

}

#endif
