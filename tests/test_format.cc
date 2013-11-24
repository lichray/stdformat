#include "../format.h"

#include "assertions.h"

using stdex::format;

int main()
{
	assert(format("") == "");
	// gcc fails
	assert(format(std::allocator<char>(), "") == "");

	assert(format("test") == "test");
	assert(format("{{}}") == "{}");

	assert_throw(std::invalid_argument, format("last{"));
	assert_throw(std::invalid_argument, format("last}"));
	assert_throw(std::invalid_argument, format("any}where"));

	assert(format("", 3.1415926) == "");
}
