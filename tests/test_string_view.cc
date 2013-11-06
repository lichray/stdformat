#include "../string_view.h"

using stdex::string_view;

int main()
{
	static_assert(std::is_same
	    <
		string_view::iterator::value_type,
		char
	    >{}, "horrible stdlib implementation");

	string_view sv;
}
