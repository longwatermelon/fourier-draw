CXX=g++
CXXFLAGS=-std=c++17 -ggdb -Wall
LDFLAGS=-lSDL2

.PHONY: all

all:
	$(CXX) $(CXXFLAGS) main.cpp $(LDFLAGS)

