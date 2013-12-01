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

#ifndef _STDEX_OSTREAM_FORMAT_H
#define _STDEX_OSTREAM_FORMAT_H

#include "format.h"

#include <ios>
#include <memory>

namespace stdex {

template <typename CharT, typename Traits, typename Allocator>
struct ostream_format
{
	using char_type = CharT;
	using traits_type = Traits;
	using ostream_type = std::basic_ostream<CharT, Traits>;
	using allocator_type = Allocator;

	explicit ostream_format(ostream_type& out) :
		ostream_format(out, allocator_type())
	{}

	explicit ostream_format(ostream_type& out, allocator_type const& a) :
		out_(out), buf_(a)
	{
		buf_.reserve(detail::pow2_roundup(80));
	}

	allocator_type get_allocator() const noexcept
	{
		return buf_.get_allocator();
	}

	template <typename... T>
	bool operator()(basic_string_view<CharT> fmt, T const&... t)
	{
		assert(buf_.empty());

		try
		{
			typename ostream_type::sentry ok(out_);

			if (not ok)
				return false;

			clear_buf _{ &buf_ };

			try
			{
				detail::vsformat(buf_, fmt,
				    std::forward_as_tuple(t...));
			}
			catch (...)
			{
				setstate_and_rethrow(ostream_type::failbit);
				return false;
			}

			auto p = buf_.data();
			auto n = buf_.size();
			auto m = std::numeric_limits<std::streamsize>::max();

			if (not n)
				return true;

			while (n > m)
			{
				if (out_.rdbuf()->sputn(p, m) != m)
				{
					out_.setstate(ostream_type::badbit);
					return false;
				}

				p += m;
				n -= m;
			}

			if (out_.rdbuf()->sputn(p, n) != n)
			{
				out_.setstate(ostream_type::badbit);
				return false;
			}

			return true;
		}

		catch (...)
		{
			setstate_and_rethrow(ostream_type::badbit);
			return false;
		}
	}

private:
	using string_type = std::basic_string<CharT, Traits, Allocator>;

	void setstate_and_rethrow(typename ostream_type::iostate rd)
	{
		bool rethrow = false;

		try
		{
			out_.setstate(rd);
		}
		catch (...)
		{
			rethrow = true;
		}

		if (rethrow)
			throw;
	}

	struct clear_buf
	{
		string_type* bufptr;

		~clear_buf()
		{
			bufptr->clear();
		}
	};

	ostream_type&	out_;
	string_type	buf_;
};

template <typename Stream>
auto make_formatted(Stream& out)
{
	return ostream_format
	<
	    typename Stream::char_type,
	    typename Stream::traits_type,
	    std::allocator<typename Stream::char_type>
	>
	(out);
}

template <typename Stream, typename Allocator>
auto make_formatted(Stream& out, Allocator const& a)
{
	return ostream_format
	<
	    typename Stream::char_type,
	    typename Stream::traits_type,
	    Allocator
	>
	(out, a);
}

}

#endif
