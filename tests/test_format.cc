#include "../format.h"

#include <cassert>

using stdex::format;

int main()
{
	assert(format("") == "");
	assert(format("", 3.1415926) == "");
}
