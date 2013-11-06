#include "../string_view.h"

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

	string_view sv;
}
