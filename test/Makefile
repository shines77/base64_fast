CFLAGS += -std=c99 -O3 -Wall -Wextra -pedantic -DNDEBUG

.PHONY: all clean test

all: test_base64 benchmark

test: clean test_base64 benchmark
	./test_base64
	./benchmark

test_base64: test_base64.c ../src/base64_fast.o
	$(CC) $(CFLAGS) -o $@ $^

benchmark: benchmark.c ../src/base64_fast.o
	$(CC) $(CFLAGS) -o $@ $^ -lrt

../%:
	make -C .. $*

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f benchmark test_base64 *.o
