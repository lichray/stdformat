# ccconf test_string_view CXX=clang++ CXXFLAGS+=-std=c++11 -stdlib=libc++ -Wall -g LDFLAGS+=-stdlib=libc++
LDFLAGS  = -stdlib=libc++  
CXXFLAGS = -std=c++11 -stdlib=libc++ -Wall -g  
CXX      = clang++  

.PHONY : all clean
all : test_string_view
clean :
	rm -f test_string_view test_string_view.o tags

tags : *.h test_string_view.cc 
	ctags *.h test_string_view.cc 

test_string_view : test_string_view.o
	${CXX} ${LDFLAGS} -o test_string_view test_string_view.o
test_string_view.o: test_string_view.cc ../string_view.h
