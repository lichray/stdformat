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

#ifndef _STDEX___FORMATTER_H
#define _STDEX___FORMATTER_H

#include "string_view.h"
#include "traits_adaptors.h"

#include <stdexcept>
#include <cassert>

namespace stdex {

namespace detail {

template <typename CharT, typename Traits, typename Allocator, typename Tuple>
inline
void vsformat(std::basic_string<CharT, Traits, Allocator>&,
    basic_string_view<CharT>, Tuple);

template <int, int, int, typename>
struct write_arg_at_impl;

#define _STDEX_G(T, literal) _Generic(T{}, \
    char: literal, \
    wchar_t: L ## literal, \
    char16_t: u ## literal, \
    char32_t: U ## literal)

}

template <typename StringType>
struct format_writer
{
	using char_type = typename StringType::value_type;
	using traits_type = typename StringType::traits_type;
	using size_type = typename StringType::size_type;

#ifndef _STDEX_TESTING

private:
	template <typename CharT, typename Traits, typename Allocator,
	          typename Tuple>
	friend
	void detail::vsformat(std::basic_string<CharT, Traits, Allocator>&,
	    basic_string_view<CharT>, Tuple);

#endif

	format_writer(StringType& buf) noexcept :
		format_writer(buf, 0)
	{}

	format_writer(StringType& buf, int width, bool padding_left = false) :
		buf_(buf), old_sz_(buf_.size()),
		width_(width), padding_left_(padding_left)
	{
		assert(width_ >= 0);
	}

public:
	template <typename CharT>
	auto send(CharT ch)
		-> If_t<std::is_same<CharT, char_type>>
	{
		buf_.push_back(ch);
	}

	template <typename CharT>
	auto send(size_type n, CharT ch)
		-> If_t<std::is_same<CharT, char_type>>
	{
		buf_.append(n, ch);
	}

	void send(basic_string_view<char_type, traits_type> s)
	{
		buf_.append(s.data(), s.size());
	}

#define _G(c) _STDEX_G(char_type, c)

	void content_width_will_be(int w)
	{
		assert(old_sz_ == buf_.size());

		if (padding_left_ and w < width_)
			buf_.append(width_ - w, _G(' '));
	}

#ifndef _STDEX_TESTING

private:
	template <int, int, int, typename>
	friend
	struct detail::write_arg_at_impl;

#endif
	void justify_content()
	{
		auto w = buf_.size() - old_sz_;

		if (w < width_)
		{
			if (padding_left_)
				buf_.insert(old_sz_, width_ - w, _G(' '));
			else
				buf_.append(width_ - w, _G(' '));
		}
	}

	void padding_left()
	{
		padding_left_ = true;
	}

#undef _G

private:
	StringType&	buf_;
	size_type	old_sz_;
	int		width_;
	bool 		padding_left_;
};

#define _G(c) _STDEX_G(CharT, c)

template <typename T>
struct formatter;

template <>
struct formatter<bool>
{
	formatter() = default;

	template <typename CharT>
	explicit formatter(basic_string_view<CharT> spec)
	{
		if (spec != _G("s"))
			throw std::invalid_argument
			{
			    R"(bool format specifier should be "s")"
			};
	}

	template <typename Writer>
	void output(Writer w, bool v)
	{
		using CharT = typename Writer::char_type;

		w.content_width_will_be(v ? 4 : 5);
		w.send(v ? _G("true") : _G("false"));
	}
};

namespace detail {

template <typename IntType>
struct int_formatter
{
	template <typename Writer>
	void output(Writer w, IntType v)
	{
	}
};

template <typename UIntType>
struct uint_formatter
{
	template <typename Writer>
	void output(Writer w, UIntType v)
	{
	}
};

template <typename RealType>
struct float_formatter
{
	template <typename Writer>
	void output(Writer w, RealType v)
	{
	}
};

template <typename CharT>
struct char_formatter
{
	char_formatter() = default;

	explicit char_formatter(basic_string_view<CharT> spec)
	{
		if (spec != _G("c"))
			throw std::invalid_argument
			{
			    R"(char format specifier should be "c")"
			};
	}

	template <typename Writer>
	void output(Writer w, CharT ch)
	{
		w.content_width_will_be(1);
		w.send(ch);
	}
};

}

template <>
struct formatter<signed char> : detail::int_formatter<signed char>
{
};

template <>
struct formatter<short> : detail::int_formatter<short>
{
	using int_formatter::int_formatter;
};

template <>
struct formatter<int> : detail::int_formatter<int>
{
	using int_formatter::int_formatter;
};

template <>
struct formatter<long> : detail::int_formatter<long>
{
	using int_formatter::int_formatter;
};

template <>
struct formatter<long long> : detail::int_formatter<long long>
{
	using int_formatter::int_formatter;
};

template <>
struct formatter<unsigned char> : detail::uint_formatter<unsigned char>
{
};

template <>
struct formatter<unsigned short> : detail::uint_formatter<unsigned short>
{
	using uint_formatter::uint_formatter;
};

template <>
struct formatter<unsigned int> : detail::uint_formatter<unsigned int>
{
	using uint_formatter::uint_formatter;
};

template <>
struct formatter<unsigned long> : detail::uint_formatter<unsigned long>
{
	using uint_formatter::uint_formatter;
};

template <>
struct formatter<unsigned long long>
	: detail::uint_formatter<unsigned long long>
{
	using uint_formatter::uint_formatter;
};

template <>
struct formatter<float> : detail::float_formatter<float>
{
	using float_formatter::float_formatter;
};

template <>
struct formatter<double> : detail::float_formatter<double>
{
	using float_formatter::float_formatter;
};

template <>
struct formatter<long double> : detail::float_formatter<long double>
{
	using float_formatter::float_formatter;
};

template <>
struct formatter<char> : detail::char_formatter<char>
{
	using char_formatter::char_formatter;
};

template <>
struct formatter<wchar_t> : detail::char_formatter<wchar_t>
{
	using char_formatter::char_formatter;
};

template <>
struct formatter<char16_t> : detail::char_formatter<char16_t>
{
	using char_formatter::char_formatter;
};

template <>
struct formatter<char32_t> : detail::char_formatter<char32_t>
{
	using char_formatter::char_formatter;
};

template <typename CharT, typename Traits>
struct formatter<basic_string_view<CharT, Traits>>
{
	typedef void default_left_justified;

	formatter() = default;

	explicit formatter(basic_string_view<CharT> spec)
	{
		if (spec != _G("s"))
			throw std::invalid_argument
			{
			    R"(string format specifier should be "s")"
			};
	}

	template <typename Writer>
	void output(Writer w, basic_string_view<CharT, Traits> s)
	{
		w.content_width_will_be(s.size());
		w.send(s);
	}
};

template <typename CharT, typename Traits, typename Allocator>
struct formatter<std::basic_string<CharT, Traits, Allocator>>
	: formatter<basic_string_view<CharT, Traits>>
{
	using formatter<basic_string_view<CharT, Traits>>::formatter;
};

template <>
struct formatter<char const*> : formatter<string_view>
{
	using formatter<string_view>::formatter;
};

template <>
struct formatter<wchar_t const*> : formatter<wstring_view>
{
	using formatter<wstring_view>::formatter;
};

template <>
struct formatter<char16_t const*> : formatter<u16string_view>
{
	using formatter<u16string_view>::formatter;
};

template <>
struct formatter<char32_t const*> : formatter<u32string_view>
{
	using formatter<u32string_view>::formatter;
};

template <>
struct formatter<char*> : formatter<char const*>
{
	using formatter<char const*>::formatter;
};

template <>
struct formatter<wchar_t*> : formatter<wchar_t const*>
{
	using formatter<wchar_t const*>::formatter;
};

template <>
struct formatter<char16_t*> : formatter<char16_t const*>
{
	using formatter<char16_t const*>::formatter;
};

template <>
struct formatter<char32_t*> : formatter<char32_t const*>
{
	using formatter<char32_t const*>::formatter;
};

template <typename T>
struct formatter<T*>
{
	formatter() = default;

	template <typename CharT>
	explicit formatter(basic_string_view<CharT> spec)
	{
		if (spec != _G("p"))
			throw std::invalid_argument
			{
			    R"(pointer format specifier should be "p")"
			};
	}
};

#undef _G

}

#endif
