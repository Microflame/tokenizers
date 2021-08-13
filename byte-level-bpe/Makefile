# CXX = g++
CXX = clang++-8
CXXFLAGS = -std=c++17 -Werror -Wall -pedantic -g -fno-omit-frame-pointer -pthread -O2

all: main

main: byte-level-bpe.o main.cpp
	$(CXX) $(CXXFLAGS) byte-level-bpe.o main.cpp -o $@

byte-level-bpe.o: byte-level-bpe.cpp byte-level-bpe.hpp
	$(CXX) -c $(CXXFLAGS) byte-level-bpe.cpp -o $@

clean:
	rm -f byte-level-bpe.o main