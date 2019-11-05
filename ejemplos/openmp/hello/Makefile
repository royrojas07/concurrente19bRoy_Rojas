CC=gcc
CXX=g++
FLAGS=-g -Wall -Wextra -fopenmp
CFLAGS=$(FLAGS) -std=gnu11
CXXFLAGS=$(FLAGS) -std=c++11

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c*)
COBJECTS=$(SOURCES:.c=.o)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=$(shell basename `pwd`)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE) *.o *.dSYM *~
