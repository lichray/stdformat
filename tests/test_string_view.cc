#include "../string_view.h"

#include "assertions.h"
#include <sstream>
#include <iomanip>

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
	assert(sv0.data() == nullptr);

	string_view sv2 = "meow\0!";

	assert(sv2.length() == 4);
	assert(*sv2.rbegin() == 'w');
	assert(*sv2.begin() == sv2.front());
	assert(*sv2.rbegin() == sv2.back());
	// N3762 violation: &front() is ill-formed.
	assert(sv2.data() == &*sv2.begin());

	assert_throw(std::out_of_range, sv2.at(4));

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
	assert(sv1.to_string() == s);

	assert(sv1.find('\0') == 4);
	assert(sv1.find('?') == string_view::npos);
	assert(sv1.find('\0', 5) == string_view::npos);
	assert(sv1.find('\0', 100) == string_view::npos);
	assert(sv1.find('\0') == sv1.find_first_of('\0'));

	assert(sv1.find_first_of("oe") == 1);
	assert(sv1.find_first_of("eo") == 1);
	assert(sv1.find_first_of("oe", 3, 2) == string_view::npos);
	assert(sv1.find_first_of("oe", 3, 3) == 4);
	assert(sv1.find_first_of("oe", 100, 2) == string_view::npos);
	assert(sv1.find_first_of(".?") == string_view::npos);
	assert(sv1.find_first_of("\0?"_sv) == 4);
	assert(sv1.find_first_of("", 2, 0) == string_view::npos);

	assert(sv1.find("ow") == 2);
	assert(sv1.find("wo") == string_view::npos);
	assert(sv1.find("ow", 3, 2) == string_view::npos);
	assert(sv1.find("ow", 100, 2) == string_view::npos);
	assert(sv1.find("ow\0"_sv, 1) == 2);
	assert(sv1.find("") == 0);
	assert(sv1.find(""_sv, sv1.length()) == sv1.length());
	assert(sv1.find(""_sv, sv1.length() + 1) == string_view::npos);

	assert(sv1.find_first_not_of('\0') == 0);
	assert(sv1.find_first_not_of('m') == 1);
	assert(sv1.find_first_not_of('\0', 4) == 5);
	assert(sv1.find_first_not_of('\0', 100) == string_view::npos);
	assert(sv0.find_first_not_of('\0') == string_view::npos);
	assert("aaaa"_sv.find_first_not_of('a') == string_view::npos);

	assert(sv1.find_first_not_of("meo") == 3);
	assert(sv1.find_first_not_of("oem") == 3);
	assert(sv1.find_first_not_of("oem", 100, 2) == string_view::npos);
	assert(sv1.find_first_not_of(sv1) == string_view::npos);
	assert(sv1.find_first_not_of("", 2, 0) == 2);
	assert(sv1.find_first_not_of(""_sv, sv1.length()) == string_view::npos);

	std::stringstream ss;
	ss << sv1;
	assert(ss.str() == sv1);

	using namespace std;

	ss.str("");
	ss << setw(7) << sv1 << '1';
	assert(ss.str() == " " + sv1.to_string() + '1');

	ss.str("");
	ss << left << setfill('0') << setw(8) << sv1;
	assert(ss.str() == sv1.to_string() + "00");

	ss.str("");
	ss << setw(-1) << sv1;
	assert(ss.str() == sv1);
}
