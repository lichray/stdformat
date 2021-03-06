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

#ifndef _STDEX_STRING_VIEW_H
#define _STDEX_STRING_VIEW_H

#include <string>
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <iosfwd>

namespace stdex {

namespace detail {

template <typename Container>
#if defined(__GLIBCXX__)
using iter = __gnu_cxx::__normal_iterator
	<typename Container::const_pointer, Container>;
#else
using iter = typename Container::const_pointer;
#endif

}

using namespace std::placeholders;

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct basic_string_view
{
	using traits_type = Traits;
	using value_type = typename Traits::char_type;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using pointer = value_type*;
	using const_pointer = value_type const*;
	using reference = value_type&;
	using const_reference = value_type const&;

	using iterator = detail::iter<basic_string_view>;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr auto npos = size_type(-1);

	constexpr basic_string_view() noexcept
		: it_(), sz_()
	{}

	template <typename Allocator>
	basic_string_view(std::basic_string
	    <CharT, Traits, Allocator> const& str) noexcept
		: basic_string_view(str.data(), str.size())
	{}

	basic_string_view(CharT const* str)
		: basic_string_view(str, traits_type::length(str))
	{}

	constexpr basic_string_view(CharT const* str, size_type len)
		: it_(str), sz_(len)
	{}

	constexpr basic_string_view(basic_string_view const&) noexcept
		= default;
	basic_string_view& operator=(basic_string_view const&) noexcept
		= default;

	constexpr iterator begin() const noexcept
	{
		return iterator(it_);
	}

	constexpr iterator end() const noexcept
	{
		return iterator(it_ + sz_);
	}

	constexpr const_iterator cbegin() const noexcept
	{
		return begin();
	}

	constexpr const_iterator cend() const noexcept
	{
		return end();
	}

	reverse_iterator rbegin() const noexcept
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend() const noexcept
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	constexpr size_type size() const noexcept
	{
		return sz_;
	}

	constexpr size_type length() const noexcept
	{
		return size();
	}

	constexpr bool empty() const noexcept
	{
		return size() == 0;
	}

	// N4023 violations: element access return rvalues.

	constexpr CharT operator[](size_type pos) const
	{
		return it_[pos];
	}

	constexpr CharT at(size_type pos) const
	{
		return pos < size() ? (*this)[pos] :
		    throw std::out_of_range("basic_string_view::at");
	}

	constexpr CharT front() const
	{
		return (*this)[0];
	}

	constexpr CharT back() const
	{
		return (*this)[size() - 1];
	}

	constexpr CharT const* data() const noexcept
	{
		return it_;
	}

	void clear() noexcept
	{
		sz_ = 0;
	}

	void remove_prefix(size_type n)
	{
		it_ += n;
		sz_ -= n;
	}

	void remove_suffix(size_type n)
	{
		sz_ -= n;
	}

	void swap(basic_string_view& sv) noexcept
	{
		std::swap(it_, sv.it_);
		std::swap(sz_, sv.sz_);
	}

	template <typename Allocator>
	explicit operator std::basic_string<CharT, Traits, Allocator>() const
	{
		return std::basic_string<CharT, Traits, Allocator>(
		    data(), size());
	}

	template <typename Allocator = std::allocator<CharT>>
	std::basic_string<CharT, Traits, Allocator> to_string(
	    Allocator const& a = Allocator()) const
	{
		return std::basic_string<CharT, Traits, Allocator>(
		    data(), size(), a);
	}

	size_type copy(CharT* s, size_type n, size_type pos = 0) const
	{
		if (pos > size())
			throw std::out_of_range("basic_string_view::copy");

		auto rlen = std::min(n, size() - pos);

		std::copy_n(begin() + pos, rlen, s);
		return rlen;
	}

	constexpr basic_string_view substr(size_type pos = 0,
	    size_type n = npos) const
	{
		return { pos <= size() ? data() + pos :
		    throw std::out_of_range("basic_string_view::substr"),
		    std::min(n, size() - pos) };
	}

	size_type find(basic_string_view s, size_type pos = 0) const noexcept
	{
		return find(s.data(), pos, s.size());
	}

	size_type find(CharT const* s, size_type pos, size_type n) const
	{
		// avoid overflow
		if (pos > size() || pos > size() - n)
			return npos;

		if (n == 0)
			return pos;

		auto it = std::search(begin() + pos, end(), s, s + n,
		    traits_eq());

		return offset_from_begin(it);
	}

	size_type find(CharT const* s, size_type pos = 0) const
	{
		return find(s, pos, traits_type::length(s));
	}

	size_type find(CharT ch, size_type pos = 0) const noexcept
	{
		if (pos >= size())
			return npos;

		auto p = traits_type::find(data() + pos, size() - pos, ch);

		if (p == nullptr)
			return npos;
		else
			return p - data();
	}

	size_type find_first_of(basic_string_view s,
	    size_type pos = 0) const noexcept
	{
		return find_first_of(s.data(), pos, s.size());
	}

	size_type find_first_of(CharT const* s, size_type pos,
	    size_type n) const
	{
		if (pos >= size())
			return npos;

		auto it = std::find_first_of(begin() + pos, end(), s, s + n,
		    traits_eq());

		return offset_from_begin(it);
	}

	size_type find_first_of(CharT const* s, size_type pos = 0) const
	{
		return find_first_of(s, pos, traits_type::length(s));
	}

	size_type find_first_of(CharT ch, size_type pos = 0) const
	{
		return find(ch, pos);
	}

	size_type find_first_not_of(basic_string_view s,
	    size_type pos = 0) const noexcept
	{
		return find_first_not_of(s.data(), pos, s.size());
	}

	size_type find_first_not_of(CharT const* s, size_type pos,
	    size_type n) const
	{
		if (pos >= size())
			return npos;

		auto it = std::find_if(begin() + pos, end(),
		    [=](CharT c)
		    {
			return std::none_of(s, s + n,
			    std::bind(traits_eq(), c, _1));
		    });

		return offset_from_begin(it);
	}

	size_type find_first_not_of(CharT const* s, size_type pos = 0) const
	{
		return find_first_not_of(s, pos, traits_type::length(s));
	}

	size_type find_first_not_of(CharT ch, size_type pos = 0) const
	{
		if (pos >= size())
			return npos;

		auto it = std::find_if_not(begin() + pos, end(),
		    std::bind(traits_eq(), ch, _1));

		return offset_from_begin(it);
	}

	friend
	bool operator==(basic_string_view a, basic_string_view b)
	{
		return a.size() == b.size() and traits_type::compare(a.data(),
		    b.data(), a.size()) == 0;
	}

	friend
	bool operator!=(basic_string_view a, basic_string_view b)
	{
		return !(a == b);
	}

private:
	struct traits_eq
	{
		constexpr bool operator()(CharT x, CharT y) const noexcept
		{
			return traits_type::eq(x, y);
		}
	};

	size_type offset_from_begin(iterator it) const
	{
		if (it == end())
			return npos;
		else
			return it - begin();
	}

	const_pointer it_;
	size_type sz_;
};

template <typename CharT, typename Traits>
inline
void swap(basic_string_view<CharT, Traits>& a,
    basic_string_view<CharT, Traits>& b) noexcept
{
	a.swap(b);
}

template <typename CharT, typename Traits>
inline
auto operator<<(std::basic_ostream<CharT, Traits>& out,
    basic_string_view<CharT, Traits> s) -> decltype(out)
{
#if defined(__GLIBCXX__)
	return __ostream_insert(out, s.data(), s.size());
#else
	typedef std::basic_ostream<CharT, Traits> ostream_type;

	typename ostream_type::sentry ok(out);

	if (not ok)
		return out;

	try
	{
		decltype(out.width()) w = 0;
		decltype(out.width()) n = s.size();

		if (out.width() > n)
		{
			w = out.width() - n;

			if ((out.flags() & ostream_type::adjustfield) !=
			    ostream_type::left)
				w = -w;
		}

		if (w == 0)
		{
			if (out.rdbuf()->sputn(s.data(), n) != n)
			{
				out.setstate(ostream_type::badbit);
				return out;
			}
		}
		else
		{
			auto c = out.fill();

			for (; w < 0; ++w)
			{
				if (Traits::eq_int_type(out.rdbuf()->sputc(c),
				    Traits::eof()))
				{
					out.setstate(ostream_type::badbit);
					return out;
				}
			}

			if (out.rdbuf()->sputn(s.data(), n) != n)
			{
				out.setstate(ostream_type::badbit);
				return out;
			}

			for (; w > 0; --w)
			{
				if (Traits::eq_int_type(out.rdbuf()->sputc(c),
				    Traits::eof()))
				{
					out.setstate(ostream_type::badbit);
					return out;
				}
			}
		}

		out.width(0);
		return out;
	}
	catch (...)
	{
		out.setstate(ostream_type::badbit);
		return out;
	}
#endif
}

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

inline namespace literals {
inline namespace string_literals {

constexpr string_view operator"" _sv(char const* str, std::size_t len)
{
	return string_view(str, len);
}

constexpr wstring_view operator"" _sv(wchar_t const* str, std::size_t len)
{
	return wstring_view(str, len);
}

constexpr u16string_view operator"" _sv(char16_t const* str, std::size_t len)
{
	return u16string_view(str, len);
}

constexpr u32string_view operator"" _sv(char32_t const* str, std::size_t len)
{
	return u32string_view(str, len);
}

}
}

}

#endif
