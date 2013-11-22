#include "../format.h"

#include <cassert>

using stdex::format;

int main()
{
	assert(format("") == "");
	// gcc fails
	assert(format(std::allocator<char>(), "") == "");

	assert(format("", 3.1415926) == "");
}
