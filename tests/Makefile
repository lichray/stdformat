# ccconf test_string_view CXX=clang++ CXXFLAGS+=-std=c++11 -stdlib=libc++ -Wall -g LDFLAGS+=-stdlib=libc++
LDFLAGS  = -stdlib=libc++  
CXXFLAGS = -std=c++11 -stdlib=libc++ -Wall -g  
CXX      = clang++  

.PHONY : all clean
all : test_format test_format_writer test_string_view
clean :
	rm -f test_format test_format.o
	rm -f test_format_writer test_format_writer.o
	rm -f test_string_view test_string_view.o

test_format : test_format.o
	${CXX} ${LDFLAGS} -o test_format test_format.o
test_format.o: test_format.cc ../format.h ../__formatter.h \
  ../string_view.h ../traits_adaptors.h
test_format_writer : test_format_writer.o
	${CXX} ${LDFLAGS} -o test_format_writer test_format_writer.o
test_format_writer.o: test_format_writer.cc ../__formatter.h \
  ../string_view.h ../traits_adaptors.h
test_string_view : test_string_view.o
	${CXX} ${LDFLAGS} -o test_string_view test_string_view.o
test_string_view.o: test_string_view.cc ../string_view.h
