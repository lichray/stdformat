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

template <typename StringType, typename Codecvt = void>
struct format_writer
{
	using char_type = typename StringType::value_type;
	using traits_type = typename StringType::traits_type;

#ifndef _STDEX_TESTING

private:
	template
	<
	    typename CharT, typename Traits, typename Allocator,
	    typename Tuple
	>
	friend
	auto vformat(Allocator const&, basic_string_view<CharT>, Tuple)
		-> std::basic_string<CharT, Traits, Allocator>;

#endif

	format_writer(StringType& buf) noexcept :
		format_writer(buf, false, 0)
	{}

	format_writer(StringType& buf, bool padding_left, int width) :
		buf_(buf), old_sz_(buf_.size()),
		padding_left_(padding_left), width_(width)
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

	void send(basic_string_view<char_type, traits_type> s)
	{
		buf_.append(s.data(), s.size());
	}

	void content_width_will_be(int w)
	{
		assert(old_sz_ == buf_.size());

		if (padding_left_)
		{
			if (w < width_)
				buf_.append(width_ - w, ' ');
			width_ = 0;
		}
	}

	void align_content()
	{
		auto w = buf_.size() - old_sz_;

		if (w < width_)
		{
			if (padding_left_)
				buf_.insert(old_sz_, width_ - w, ' ');
			else
				buf_.append(width_ - w, ' ');
		}

		width_ = 0;
	}

private:
	StringType&			buf_;
	typename StringType::size_type	old_sz_;
	bool 				padding_left_;
	int				width_;
};

template <typename T>
struct formatter;

template <>
struct formatter<bool>
{
	formatter() = default;

	template <typename CharT>
	formatter(basic_string_view<CharT> spec)
	{
		if (spec != "s")
			throw std::invalid_argument(std::string(spec));
	}
};

template <typename T>
struct formatter<T*>
{
	formatter() = default;

	template <typename CharT>
	formatter(basic_string_view<CharT> spec)
	{
		if (spec != "p")
			throw std::invalid_argument(std::string(spec));
	}
};

}

#endif
