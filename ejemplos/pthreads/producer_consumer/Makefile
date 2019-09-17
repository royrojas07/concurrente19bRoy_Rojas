APPNAME=$(shell basename $(shell pwd))

CC=cc
CXX=g++
FLAGS=-g -Wall -Wextra
CFLAGS=$(FLAGS)
CXXFLAGS=$(FLAGS)
LIBS=-pthread
CLIBS=$(LIBS)
CXXLIBS=$(LIBS)

$(APPNAME): $(APPNAME).c
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

.PHONY: ignore
ignore:
	echo $(APPNAME) >> .gitignore

.PHONY: clean
clean:
	rm -f $(APPNAME)
