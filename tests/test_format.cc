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

	assert_throw(std::invalid_argument, format("{} {1}", 'a', 'a'));
	assert_throw(std::invalid_argument, format("{1} {}", 'a', 'a'));

	assert_throw(std::out_of_range, format("{}"));
	assert_throw(std::invalid_argument, format("{0}"));
	assert_throw(std::out_of_range, format("{2}", 'a'));
	assert_throw(std::out_of_range, format("{} {} {}", 'a', 'a'));

	assert(format("{} {}", true, false) == "true false");
	assert(format("{2} {3} {1}", 'a', 'b', 'c') == "b c a");

	assert_throw(std::invalid_argument, format("{:0.4d"));
	assert_throw(std::invalid_argument, format("{0:"));

	assert(format("{:}", std::string("alt")) == "alt");
	assert(format("{:3}|{:10}", '1', "hello") == "  1|hello     ");
	assert(format("{:<3}|{:>10}", '1', "hello") == "1  |     hello");
	assert(format("{2:<3}|{1:10}", '1', "hello") == "hello|         1");
}
