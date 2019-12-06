CC=gcc
CXX=g++
FLAGS=-g -Wall -Wextra -fopenmp
CFLAGS=$(FLAGS)
CXXFLAGS=$(FLAGS)
LIBS=-lm

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c*)
COBJECTS=$(SOURCES:.c=.o)
OBJECTS=$(COBJECTS:.cpp=.o)
EXECUTABLE=$(shell basename `pwd`)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE) *.o *.dSYM *~
