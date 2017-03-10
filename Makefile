CFLAGS=-g -O0 -W -Wall -std=c11
LDFLAGS=
DEPS=add.o
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
