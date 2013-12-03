#include "../ostream_format.h"

#include "assertions.h"

#include <sstream>
#include <iostream>


int main()
{
	std::ostringstream os;
	auto trace = stdex::make_formatted(os);

	assert(trace("", "unused"));
	assert(os.str() == "");

	assert(trace("{:c} {:s}", 'a', true));
	assert(os.str() == "a true");

	assert(trace("{1:s}|{1:5s}", "str", "unused"));
	assert(os.str() == "a truestr|str  ");

	std::stringstream ss;
	auto challege = stdex::make_formatted(ss, std::allocator<char>());

	ss.exceptions(std::ios_base::badbit);

	assert(!challege("{}"));
	assert(ss.rdstate() == std::ios_base::failbit);

	ss.clear();
	ss.exceptions(std::ios_base::failbit);

	assert_throw(std::invalid_argument, challege("{1:"));
	assert(ss.rdstate() == std::ios_base::failbit);

	auto wprintf = stdex::make_formatted(std::wcout);

	wprintf(L"hello, {}\n", L"world");
}
