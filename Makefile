CC=/usr/bin/g++

CFLAGS=-Wall
LDFLAGS=

a1: main.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean
clean:
	rm -rf *.o a1
