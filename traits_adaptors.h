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

#ifndef _STDEX_HOTT_H
#define _STDEX_HOTT_H

#include <type_traits>

namespace stdex {

template <template <typename> class F, template <typename> class... Fs>
struct composed
{
	template <typename T>
	using call = F
		<
		    typename composed<Fs...>::template call<T>::type
		>;

	template <typename T, typename... Ts>
	using apply = typename call<T>::template apply<Ts...>;
};

template <template <typename> class F, template <typename> class G>
struct composed<F, G>
{
	template <typename T>
	using call = F
		<
		    typename G<T>::type
		>;

	template <typename T, typename... Ts>
	using apply = typename call<T>::template apply<Ts...>;
};

namespace detail {

template <int N, template <typename...> class F, typename... As>
struct currying
{
	template <typename T>
	using call = currying<N - 1, F, As..., T>;

	template <typename... Ts>
	using apply = F<As..., Ts...>;
};

template <template <typename...> class F, typename... As>
struct currying<0, F, As...> : F<As...>
{
	// alias does not work; might be a compiler bug
	template <typename... Ts>
	struct apply : F<As..., Ts...> {};
};

template <int N, typename F, typename T2>
struct bind
{
	template <typename T1>
	using call = bind<N - 1, bind<2, F, T1>, T2>;

	template <typename T, typename... Ts>
	using apply = typename call<T>::template apply<Ts...>;
};

template <typename F, typename T2>
struct bind<2, F, T2>
{
	template <typename T1>
	using call = typename F::template call<T1>::template call<T2>;

	template <typename T, typename... Ts>
	using apply = typename call<T>::template apply<Ts...>;
};

template <bool, typename T, typename... U>
struct lazy_conditional_c
{
	static_assert(sizeof...(U) <= 1, "two branches");
};

// lazy_enable_if_c
template <typename T>
struct lazy_conditional_c<true, T>
{
	using type = typename T::type;
};

template <typename T, typename U>
struct lazy_conditional_c<true, T, U>
{
	using type = typename T::type;
};

template <typename T, typename U>
struct lazy_conditional_c<false, T, U>
{
	using type = typename U::type;
};

}

template <template <typename...> class F, int N = 2>
using curried = detail::currying<N, F>;

template <typename F, int N = 2>
struct flipped
{
	template <typename T>
	using call = detail::bind<N, F, T>;

	template <typename T, typename... Ts>
	using apply = typename call<T>::template apply<Ts...>;
};

template <bool V>
using bool_constant = std::integral_constant<bool, V>;

template <typename X>
struct Not : bool_constant
	<
	    not X::value
	>
{};

template <typename X, typename... Xs>
struct and_also : bool_constant
	<
	    X::value and and_also<Xs...>::value
	>
{};

template <typename X, typename Y>
struct and_also<X, Y> : bool_constant
	<
	    X::value and Y::value
	>
{};

template <typename X, typename... Xs>
struct or_else : bool_constant
	<
	    X::value or or_else<Xs...>::value
	>
{};

template <typename X, typename Y>
struct or_else<X, Y> : bool_constant
	<
	    X::value or Y::value
	>
{};

template <template <typename> class F>
struct negatively
{
	template <typename T>
	using call = Not<F<T>>;
};

template <template <typename> class F, template <typename> class... Fs>
struct both
{
	template <typename T>
	using call = and_also<F<T>, typename both<Fs...>::template call<T>>;
};

template <template <typename> class F, template <typename> class G>
struct both<F, G>
{
	template <typename T>
	using call = and_also<F<T>, G<T>>;
};

template <template <typename> class F, template <typename> class... Fs>
struct either
{
	template <typename T>
	using call = or_else<F<T>, typename both<Fs...>::template call<T>>;
};

template <template <typename> class F, template <typename> class G>
struct either<F, G>
{
	template <typename T>
	using call = or_else<F<T>, G<T>>;
};

template <template <typename> class... Fs>
struct neither : negatively<either<Fs...>::template call> {};

template <typename T>
struct identity_of
{
	using type = T;
};

template <typename V, typename T, typename... U>
struct If : detail::lazy_conditional_c<V::value, T, U...> {};

template <typename V, typename T = identity_of<void>, typename... U>
using If_t = typename If<V, T, U...>::type;

template <bool V, typename T = identity_of<void>, typename... U>
using If_ct = typename detail::lazy_conditional_c<V, T, U...>::type;

}

#endif
