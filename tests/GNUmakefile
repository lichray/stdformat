# ccconf test_string_view CXX=g++48 CXXFLAGS+=-std=c++11 -Wall -g
CXXFLAGS = -std=c++11 -Wall -g  
CXX      = g++48  

.PHONY : all clean
all : test_string_view
clean :
	rm -f test_string_view test_string_view.o tags

tags : *.h test_string_view.cc 
	ctags *.h test_string_view.cc 

test_string_view : test_string_view.o
	${CXX} ${LDFLAGS} -o test_string_view test_string_view.o
test_string_view.o: test_string_view.cc ../string_view.h
