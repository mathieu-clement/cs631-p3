CFLAGS=-g -O0 -W -Wall -Werror -std=c11
LDFLAGS=
DEPS=add.o
OBJS=armemu.o $(DEPS)
PROG=armemu
TOOLCHAIN=/usr/local/linaro/arm-linux-gnueabihf-raspbian/bin
CC=$(TOOLCHAIN)/arm-linux-gnueabihf-gcc
AS=$(TOOLCHAIN)/arm-linux-gnueabihf-as

all: $(PROG)

$(PROG): $(OBJS)

armemu.o : armemu.c add.o

%.o : %.s
	$(AS) -o $@ $<

.PHONY : clean

clean:
	rm -f $(OBJS) $(PROG)
