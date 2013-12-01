#include "../ostream_format.h"

#include "assertions.h"

#include <sstream>
#include <iostream>


int main()
{
	std::ostringstream os;
	stdex::ostream_format<char> trace(os);

	assert(trace("", "unused"));
	assert(os.str() == "");

	assert(trace("{:c} {:s}", 'a', true));
	assert(os.str() == "a true");

	assert(trace("{1:s}|{1:5s}", "str", "unused"));
	assert(os.str() == "a truestr|str  ");

	std::stringstream ss;
	stdex::ostream_format<char> challege(ss, trace.get_allocator());

	ss.exceptions(std::ios_base::badbit);

	assert(!challege("{}"));
	assert(ss.rdstate() == std::ios_base::failbit);

	ss.clear();
	ss.exceptions(std::ios_base::failbit);

	assert_throw(std::invalid_argument, challege("{1:"));
	assert(ss.rdstate() & std::ios_base::failbit);
	assert(ss.rdstate() & std::ios_base::badbit);

	stdex::ostream_format<wchar_t> wprintf(std::wcout);

	wprintf(L"hello, {}\n", L"world");
}
