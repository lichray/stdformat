#include "../string_view.h"

#include <cassert>

using stdex::string_view;

int main()
{
	static_assert(std::is_same
	    <
		std::iterator_traits<string_view::iterator>::value_type,
		char
	    >{}, "broken stdlib");

	static_assert(std::is_same
	    <
		std::iterator_traits<string_view::iterator>::reference,
		char const&
	    >{}, "broken stdlib");

	static_assert(std::is_same
	    <
		string_view::reverse_iterator::difference_type,
		string_view::difference_type
	    >{}, "broken stdlib");

	static_assert(std::is_nothrow_default_constructible<string_view>{}, "");
	static_assert(std::is_nothrow_move_constructible<string_view>{}, "");
	static_assert(std::is_nothrow_move_assignable<string_view>{}, "");

	constexpr string_view sv;

	static_assert(sv.empty(), "");
	assert(sv.length() == string_view(std::string()).size());

	string_view sv2 = "meow\0!";

	assert(sv2.length() == 4);
	assert(*sv2.rbegin() == 'w');
}
