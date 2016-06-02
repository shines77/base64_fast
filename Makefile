CFLAGS += -std=c99 -O3 -Wall -Wextra -pedantic -DNDEBUG

# Set OBJCOPY if not defined by environment:
OBJCOPY ?= objcopy

OBJS =

.PHONY: all analyze clean test

all: bin/base64 test/benchmark

test: clean test/benchmark
	./test/benchmark

bin/base64: src/base64.o src/base64_fast.o
	$(CC) $(CFLAGS) -o $@ $^

# src/base64_fast.o: $(OBJS)
#	$(LD) --relocatable -o $@ $^
#	$(OBJCOPY) --keep-global-symbols=lib/exports.txt $@

test/benchmark: test/benchmark.o src/base64_fast.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

analyze: clean
	scan-build --use-analyzer=`which clang` --status-bugs make

clean:
	rm -f bin/base64 test/benchmark src/base64.o src/base64_fast.o test/benchmark.o $(OBJS)
