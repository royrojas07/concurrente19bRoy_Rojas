APPNAME=$(shell basename $(shell pwd))

$(APPNAME): $(APPNAME).c
	cc -g -Wall -Wextra $(APPNAME).c -o $(APPNAME) -pthread

.PHONY: ignore
ignore:
	echo $(APPNAME) >> .gitignore

.PHONY: clean
clean:
	rm -f $(APPNAME)
