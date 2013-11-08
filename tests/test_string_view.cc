#include "../string_view.h"

#include <cassert>

using stdex::string_view;
using namespace stdex::string_literals;

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

	constexpr string_view sv0;

	static_assert(sv0.empty(), "");
	assert(sv0.length() == string_view(std::string()).size());

	string_view sv2 = "meow\0!";

	assert(sv2.length() == 4);
	assert(*sv2.rbegin() == 'w');
	assert(*sv2.begin() == sv2.front());
	assert(*sv2.rbegin() == sv2.back());
	// N3762 violation: &front() is ill-formed.
	assert(sv2.data() == &*sv2.begin());

	try
	{
		sv2.at(4);
		assert(0);
	}
	catch (std::exception&)
	{
		assert(1);
	}

	auto sv1 = sv2;
	sv1.remove_prefix(2);
	sv1.remove_suffix(1);
	swap(sv1, sv2);

	assert(sv1 == "meow");
	assert(sv2 == "o");
	assert(sv2.back() == sv2.front());
	assert(sv2 == sv1.substr(2, 1));

	sv2.clear();
	assert(sv2 == sv0);

	sv1 = "meow\0!"_sv;
	char sb[10];
	auto xlen = sv1.copy(sb, sizeof(sb));
	auto s = std::string(sv1);

	assert(xlen == 6);
	assert(s.length() == xlen);
	assert(sv1 == s);
	assert(sv1 != sb);
}
