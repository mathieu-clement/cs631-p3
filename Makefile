DEBUG=1
ITERS=10000
CFLAGS=-g -O0 -W -Wall -Werror -std=c11 -DDEBUG=$(DEBUG) -DITERS=$(ITERS)
LDFLAGS=
DEPS=add_function.o sum_array.o fib_iter.o fib_rec.o find_max.o find_str.o state.o analysis.o data_processing.o branch.o memory.o conditions.o bits.o
REPORT_DIR=reports
OBJS=armemu.o $(DEPS)
PROG=armemu
CC=clang
AS=as

all: $(PROG)

$(PROG): $(OBJS)

armemu.o : armemu.c 

%.o : %.s
	$(AS) -o $@ $<

.PHONY : clean create_report_dir

reports: $(PROG) create_report_dir find_max.txt sum_array.txt fib_iter.txt fib_rec.txt find_str.txt

create_report_dir: 
	mkdir $(REPORT_DIR) || true

find_max.txt: 
	./armemu find_max > $(REPORT_DIR)/$@

sum_array.txt:
	./armemu sum_array > $(REPORT_DIR)/$@

fib_iter.txt:
	./armemu fib_iter 5 > $(REPORT_DIR)/$@

fib_rec.txt:
	./armemu fib_rec 5 > $(REPORT_DIR)/$@

find_str.txt:
	./armemu "Les chaussettes de l'archi-duchesse sont-elles seches et archi-seches" "archibald" > $(REPORT_DIR/$@

clean:
	rm -rf $(OBJS) $(PROG) $(REPORT_DIR)/
