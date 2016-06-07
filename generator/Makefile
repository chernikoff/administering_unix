all: generator

generator: generator.o
	g++ -g generator.o -o generator

generator.o: generator.cpp
	g++ -g -std=c++11 -c generator.cpp

install: generator
	install -m 0755 generator /usr/bin/generator
