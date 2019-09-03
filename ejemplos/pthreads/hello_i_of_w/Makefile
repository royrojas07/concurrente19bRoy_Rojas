APPNAME=hello_i_of_w

$(APPNAME): $(APPNAME).c
	cc -g -Wall -Wextra $(APPNAME).c -o $(APPNAME) -pthread

.PHONY: clean
clean:
	rm -f $(APPNAME)
