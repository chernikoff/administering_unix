all: deleter

deleter: deleter.o
	g++ -g deleter.o -lpthread -o deleter

deleter.o:
	g++ -g -std=c++14 -O2 -c deleter.cpp

clean:
	rm deleter.o deleter
