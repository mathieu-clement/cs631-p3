DEBUG=0
SINGLE_STEP_MODE=0
CFLAGS=-g -O3 -W -Wall -Werror -std=c11 -DDEBUG=$(DEBUG) -DSINGLE_STEP_MODE=$(SINGLE_STEP_MODE)
LDFLAGS=
DEPS=sum_array.o fib_iter.o fib_rec.o find_max.o find_str.o state.o analysis.o data_processing.o branch.o memory.o conditions.o bits.o
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
	echo "find_max 1 2 3 4 5 6 7 8 9 10" > $(REPORT_DIR)/$@
	ITERS=100000 ./armemu find_max 1 2 3 4 5 6 7 8 9 10 >> $(REPORT_DIR)/$@

sum_array.txt:
	echo "sum_array 1 2 3 4 5 6 7 8 9 10" > $(REPORT_DIR)/$@
	ITERS=100000 ./armemu sum_array 1 2 3 4 5 6 7 8 9 10 >> $(REPORT_DIR)/$@

fib_iter.txt:
	echo "fib_iter 5 1 2 3 4 5 6 7 8 9 10" > $(REPORT_DIR)/$@
	ITERS=100000 ./armemu fib_iter 5 >> $(REPORT_DIR)/$@

fib_rec.txt:
	echo "fib_rec 5" > $(REPORT_DIR)/$@
	ITERS=100000 ./armemu fib_rec 5 >> $(REPORT_DIR)/$@

find_str.txt:
	echo "find_str \"Les chaussettes...\" \"archibald\"" > $(REPORT_DIR)/$@
	ITERS=10000 ./armemu find_str "Les chaussettes de l'archi-duchesse sont-elles seches et archi-seches" "archibald" > $(REPORT_DIR)/$@

clean:
	rm -rf $(OBJS) $(PROG) $(REPORT_DIR)/
