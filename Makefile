CFLAGS=-g -O0 -W -Wall -Werror -std=c11
LDFLAGS=
DEPS=add_function.o sum_array.o
OBJS=armemu.o $(DEPS)
PROG=armemu
CC=clang
AS=as

all: $(PROG)

$(PROG): $(OBJS)

armemu.o : armemu.c

%.o : %.s
	$(AS) -o $@ $<

.PHONY : clean

clean:
	rm -f $(OBJS) $(PROG)
