CFLAGS += -std=c99 -O3 -Wall -Wextra -pedantic

# Set OBJCOPY if not defined by environment:
OBJCOPY ?= objcopy

OBJS = \
  src/base64_fast.o

.PHONY: all analyze clean

all: bin/base64 src/base64_fast.o

bin/base64: src/base64.o src/base64_fast.o
	$(CC) $(CFLAGS) -o $@ $^

src/base64_fast.o: $(OBJS)
	$(LD) --relocatable -o $@ $^
	$(OBJCOPY) --keep-global-symbols=lib/exports.txt $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

analyze: clean
	scan-build --use-analyzer=`which clang` --status-bugs make

clean:
	rm -f bin/base64 src/base64.o src/base64_fast.o $(OBJS)
