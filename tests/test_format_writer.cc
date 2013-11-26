#define _STDEX_TESTING
#include "../__formatter.h"

#include <cassert>

using namespace stdex::string_literals;

int main()
{
	std::string ts;

	{
		stdex::format_writer<std::string> w(ts, false, 8);
		w.send("hello");
		assert(ts == "hello");
	}

	{
		stdex::format_writer<std::string> w(ts, true, 0);
		w.send(", world");
		w.align_content();
		assert(ts == "hello, world");
	}

	{
		stdex::format_writer<std::string> w(ts);
		w.content_width_will_be(1);
		w.send('!');
		w.align_content();
		assert(ts == "hello, world!");
	}

	ts.erase();

	{
		stdex::format_writer<std::string> w1(ts, false, 8);
		w1.send("hello");
		w1.send(',');
		w1.align_content();
		assert(ts == "hello,  ");

		stdex::format_writer<std::string> w2(ts, true, 8);
		w2.content_width_will_be(5);
		w2.send("world");
		w2.align_content();
		assert(ts == "hello,     world");

		w2.align_content();
		assert(ts == "hello,     world");
	}

	ts.erase();

	{
		stdex::format_writer<std::string> w1(ts, false, 3);
		w1.content_width_will_be(2);
		w1.send("1.");
		w1.align_content();
		assert(ts == "1. ");

		stdex::format_writer<std::string> w2(ts, true, 6);
		w2.send("te\0st"_sv);
		w2.align_content();
		assert(ts == "1.  te\0st"_sv);
	}
}
