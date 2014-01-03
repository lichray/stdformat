#include "../format.h"

#include "assertions.h"

using stdex::format;

struct NoSpec {};

template <>
struct stdex::formatter<NoSpec>
{
	template <typename Writer>
	void output(Writer w, NoSpec)
	{
		w.send("NoSpec");
	}
};


int main()
{
	assert(format("") == "");
	assert(format(std::allocator<char>(), "") == "");

	assert(format("", "") == "");
	assert(format<std::char_traits<char>>("", "") == "");

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

	assert_throw(std::out_of_range, format("{2147483647}"));
	assert_throw(std::overflow_error, format("{2147483648}"));

	assert(format("{} {}", true, false) == "true false");
	assert(format("{2} {3} {1}", 'a', 'b', 'c') == "b c a");

	assert_throw(std::invalid_argument, format("{:4.4d"));
	assert_throw(std::invalid_argument, format("{1:"));
	assert_throw(std::invalid_argument, format("{1:<"));

	assert(format("{:}", std::string("alt")) == "alt");
	assert(format("{:3}|{:10}", '1', "hello") == "  1|hello     ");
	assert(format("{:<3}|{:>10}", '1', "hello") == "1  |     hello");
	assert(format("{2:<3}|{1:10}", '1', "hello") == "hello|         1");

	assert_throw(std::invalid_argument, format("{s}", 'a'));
	assert_throw(std::invalid_argument, format("{c }", 'a'));

	assert(format("{:c} {:s}", 'a', true) == "a true");
	assert(format("{1:s}|{1:5s}", "str", "unused") == "str|str  ");

	assert(format(u"{:c} {:s}", u'a', true) == u"a true");
	assert(format(u"{1:s}|{1:5s}", u"str", u"unused") == u"str|str  ");

	assert(format("{:8}", NoSpec()) == "  NoSpec");
	assert_throw(std::invalid_argument, format("{:s}", NoSpec()));

	assert_throw(std::invalid_argument, format("{1:*"));
	assert_throw(std::invalid_argument, format("{1:*0d}"));

	assert(format("{:*}", 4, 'a') == "   a");
	assert(format("{:*}", -2, 'a') == "a");
	assert(format("{1:<*2}", 'a', 3U) == "a  ");

	assert_throw(std::out_of_range, format("{:*}"));
	assert_throw(std::overflow_error, format("{:*}", 2147483648LL));
	assert_throw(std::overflow_error, format("{:*}", 4294967295UL));
	assert_throw(std::underflow_error, format("{:*}", -2147483649L));
	assert_throw(std::invalid_argument, format("{:*}", 8.0));
}
