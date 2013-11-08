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

namespace stdex {

namespace detail {

template <typename Container>
#if defined(_LIBCPP_VERSION) && 0
using iter = std::__wrap_iter<typename Container::pointer>;
#elif defined(__GLIBCXX__)
using iter = __gnu_cxx::__normal_iterator
	<typename Container::pointer, Container>;
#else
using iter = typename Container::pointer;
#endif

}

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct basic_string_view
{
	using traits_type = Traits;
	using value_type = typename Traits::char_type;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using pointer = value_type const*;
	using const_pointer = pointer;
	using reference = value_type const&;
	using const_reference = reference;

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
		return it_;
	}

	constexpr iterator end() const noexcept
	{
		return it_ + sz_;
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

	// N3762 violation: no max_size().

	constexpr size_type length() const noexcept
	{
		return size();
	}

	constexpr bool empty() const noexcept
	{
		return size() == 0;
	}

	// N3762 violations: element access return rvalues.

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
		return &*it_;
	}

	void clear() noexcept
	{
		sz_ = 0;
	}

	void remove_prefix(size_type n)
	{
		if (n > size())
			throw std::out_of_range(
			    "basic_string_view::remove_prefix");

		it_ += n;
		sz_ -= n;
	}

	void remove_suffix(size_type n)
	{
		if (n > size())
			throw std::out_of_range(
			    "basic_string_view::remove_suffix");

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

	// N3762 disagreement: not noexcept since C++14

	size_type find(CharT ch, size_type pos = 0) const
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
		    [](CharT x, CharT y) { return traits_type::eq(x, y); });

		if (it == end())
			return npos;
		else
			return it - begin();
	}

	size_type find_first_of(CharT const* s, size_type pos = 0) const
	{
		return find_first_of(s, pos, traits_type::length(s));
	}

	friend inline
	bool operator==(basic_string_view a, basic_string_view b)
	{
		return a.size() == b.size() and traits_type::compare(a.data(),
		    b.data(), a.size()) == 0;
	}

	friend inline
	bool operator!=(basic_string_view a, basic_string_view b)
	{
		return !(a == b);
	}

private:
	iterator it_;
	size_type sz_;
};

template <typename CharT, typename Traits>
inline
void swap(basic_string_view<CharT, Traits>& a,
    basic_string_view<CharT, Traits>& b) noexcept(noexcept(a.swap(b)))
{
	a.swap(b);
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
