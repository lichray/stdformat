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
#include "__aux.h"

#include <tuple>
#include <functional>

namespace stdex {

namespace detail {

#define _G(c) _STDEX_G(CharT, c)

template <typename CharT>
inline
bool leads_digits(CharT ch)
{
	return _G('0') < ch and ch <= _G('9');
}

template <typename CharT>
inline
int parse_int(basic_string_view<CharT>& s)
{
	auto it = s.begin();
	int n = *it++ - _G('0');

	for (; it != s.end() and (_G('0') <= *it and *it <= _G('9')); ++it)
	{
		auto d = *it - _G('0');

		if ((std::numeric_limits<int>::max() - d) / n < 10)
			throw std::overflow_error
			{
			    "integer overflow in format"
			};

		n *= 10;
		n += d;
	}

	s.remove_prefix(it - s.begin());

	return n;
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
		using T = std::decay_t
		    <
			typename std::tuple_element<Mid - 1, Tuple>::type
		    >;

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
		    "target type accepts no format specifier"
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

template <int Low, int High, int Mid = (Low + High) / 2, typename = void>
struct arg_as_int_at_impl;

template <int Low, int High, int Mid>
struct arg_as_int_at_impl<Low, High, Mid, If_ct<(Low > High)>>
{
	template <typename Tuple>
	static
	int apply(int n, Tuple tp)
	{
		throw std::out_of_range
		{
		    "tuple index out of range"
		};
	}
};

template <int Mid>
struct arg_as_int_at_impl<Mid, Mid, Mid, void>
{
	template <typename Tuple>
	static
	int apply(int n, Tuple tp)
	{
		if (n != Mid)
			throw std::out_of_range
			{
			    "tuple index out of range"
			};

		return do_get_int(std::get<Mid - 1>(tp));
	}

private:

	template <typename T>
	static
	int do_get_int(T const& t)
	{
		return do_get_int(t,
		    is_nonarrow_convertible<T, int>(),
		    std::is_integral<T>());
	}

	template <typename T>
	static
	int do_get_int(T const& t, std::true_type, ...)
	{
		return t;
	}

	template <typename T>
	static
	int do_get_int(T const& t, std::false_type, std::true_type)
	{
		test_range(t);

		return t;
	}

	// floating point excluded
	template <typename T>
	static
	auto test_range(T t)
		-> If_t<std::is_signed<T>>
	{
		if (t < std::numeric_limits<int>::min())
			throw std::underflow_error
			{
			    "integer underflow when converting target object"
			};

		if (t > std::numeric_limits<int>::max())
			throw std::overflow_error
			{
			    "integer overflow when converting target object"
			};
	}

	// shorter unsigned excluded
	template <typename T>
	static
	auto test_range(T t)
		-> If_t<std::is_unsigned<T>>
	{
		if (t > std::numeric_limits<int>::max())
			throw std::overflow_error
			{
			    "integer overflow when converting target object"
			};
	}

	template <typename T>
	static
	int do_get_int(T const& t, std::false_type, std::false_type)
	{
		throw std::invalid_argument
		{
		    "target type cannot be used as int"
		};
	}
};

template <int Low, int High, int Mid>
struct arg_as_int_at_impl<Low, High, Mid, If_ct<(Low < High)>>
{
	template <typename Tuple>
	static
	int apply(int n, Tuple tp)
	{
		if (n < Mid)
			return arg_as_int_at_impl<Low, Mid - 1>::apply(n, tp);
		else if (n == Mid)
			return arg_as_int_at_impl<Mid, Mid>::apply(n, tp);
		else
			return arg_as_int_at_impl<Mid + 1, High>::apply(n, tp);
	}
};

template <typename Tuple>
inline
int arg_as_int_at(int n, Tuple tp)
{
	return arg_as_int_at_impl<1, std::tuple_size<Tuple>{}>::apply(n, tp);
}

template <typename CharT, typename Traits, typename Allocator, typename Tuple>
void vsformat(std::basic_string<CharT, Traits, Allocator>& buf,
              basic_string_view<CharT> fmt,
              Tuple tp)
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

	int arg_index = 0;
	bool sequential;

	while (1)
	{
		auto off = fmt.find_first_of(_G("{}"));

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

		if (ch == _G('}'))
		{
			if (fmt.empty() or fmt.front() != _G('}'))
				throw std::invalid_argument
				{
				    "Single '}' encountered in format string"
				};

			buf.push_back(_G('}'));
			fmt.remove_prefix(1);
			continue;
		}

		if (fmt.empty())
			throw std::invalid_argument
			{
			    "Single '{' encountered in format string"
			};

		if (fmt.front() == _G('{'))
		{
			buf.push_back(_G('{'));
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

		if (ch == _G(':'))
		{
			adjustment adj = adjustment::unspecified;
			int width = 0;

			expect_more();

			switch (fmt.front())
			{
			case _G('<'):
				adj = adjustment::left;
				break;
			case _G('>'):
				adj = adjustment::right;
				break;
			}

			if (adj != adjustment::unspecified)
			{
				fmt.remove_prefix(1);
				expect_more();
			}

			if (leads_digits(fmt.front()))
			{
				width = parse_int(fmt);
			}
			else if (fmt.front() == _G('*'))
			{
				fmt.remove_prefix(1);

				if (sequential)
					width = arg_as_int_at(arg_index++, tp);

				else if (fmt.empty() or
				    not leads_digits(fmt.front()))
					throw std::invalid_argument
					{
						"expecting a nonzero digit"
					};

				else
					width = arg_as_int_at(parse_int(fmt),
					    tp);

				if (width < 0)
					width = 0;
			}

			auto off = fmt.find(_G('}'));

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

		else if (ch == _G('}'))
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
}

#undef _G

template <typename CharT, typename Allocator>
struct string_from_allocator
{
	using type = std::basic_string<CharT, std::char_traits<CharT>, Allocator>;
};

}

template <typename Traits, typename Allocator, typename... T>
inline
auto format(Allocator const& a,
            basic_string_view<typename Traits::char_type> fmt,
            T const&... t)
	-> If_t
	<
	    Not
	    <
		std::is_convertible
		<
		    Allocator,
		    basic_string_view<typename Traits::char_type>
		>
	    >,
	    identity_of
	    <
		std::basic_string<typename Traits::char_type, Traits, Allocator>
	    >
	>
{
	std::basic_string
	<
	    typename Traits::char_type,
	    Traits,
	    Allocator
	> buf(a);

	buf.reserve(detail::pow2_roundup(fmt.size()));
	detail::vsformat(buf, fmt, std::forward_as_tuple(t...));

	return buf;
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

template <typename Allocator, typename... T>
inline
auto format(Allocator const& a, string_view fmt, T const&... t)
	-> If_t
	<
	    Not<std::is_convertible<Allocator, string_view>>,
	    detail::string_from_allocator<char, Allocator>
	>
{
	return format<std::string::traits_type>(a, fmt, t...);
}

template <typename Allocator, typename... T>
inline
auto format(Allocator const& a, wstring_view fmt, T const&... t)
	-> If_t
	<
	    Not<std::is_convertible<Allocator, wstring_view>>,
	    detail::string_from_allocator<wchar_t, Allocator>
	>
{
	return format<std::wstring::traits_type>(a, fmt, t...);
}

template <typename Allocator, typename... T>
inline
auto format(Allocator const& a, u16string_view fmt, T const&... t)
	-> If_t
	<
	    Not<std::is_convertible<Allocator, u16string_view>>,
	    detail::string_from_allocator<char16_t, Allocator>
	>
{
	return format<std::u16string::traits_type>(a, fmt, t...);
}

template <typename Allocator, typename... T>
inline
auto format(Allocator const& a, u32string_view fmt, T const&... t)
	-> If_t
	<
	    Not<std::is_convertible<Allocator, u32string_view>>,
	    detail::string_from_allocator<char32_t, Allocator>
	>
{
	return format<std::u32string::traits_type>(a, fmt, t...);
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
