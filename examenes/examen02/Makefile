CC=mpicc
CXX=mpic++
FLAGS=-g -Wall -Wextra -std=gnu++11
CFLAGS=$(FLAGS)
CXXFLAGS=$(FLAGS)
LIBS=
N=1

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c*)
COBJECTS=$(SOURCES:.c=.o)
OBJECTS=$(COBJECTS:.cpp=.o)
EXECUTABLE=$(shell basename `pwd`)

TEST_IN=$(wildcard tests/input*.txt)
TEST_OUT=$(TEST_IN:tests/input%.txt=%)

.SECONDEXPANSION:

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: test
test: $(EXECUTABLE) $(TEST_OUT) | $$(@D)/.
	@rm -rf $(TEST_OUT)

.PHONY: %
%: tests/input%.txt | $$(@D)/$$(@F)/.
	bash -c "icdiff --no-headers tests/output$@.txt <(mpiexec -n $N ./$(EXECUTABLE) $< 0.8 1.0)"

.PRECIOUS: %/.
%/.:
	@mkdir -p $(dir $@)

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE) *.o *.dSYM *~
