CC=/usr/bin/g++

GIT_COMMIT := $(shell git describe --always --dirty)
CFLAGS=-Wall -DGIT_COMMIT=\"$(GIT_COMMIT)\" -Ispdlog/include
LDFLAGS=-Lspdlog/build/ -lspdlog


a1: main.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean
clean:
	rm -rf *.o a1
