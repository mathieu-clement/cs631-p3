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
	export ITERS=100000
	echo > $(REPORT_DIR)/$@
	./armemu find_max 1 2 3 4 5 6 7 8 9 10 >> $(REPORT_DIR)/$@
	./armemu find_max -1 -5 -18 -333333 >> $(REPORT_DIR)/$@
	./armemu find_max 0 0 0 0 0 0 >> $(REPORT_DIR)/$@

sum_array.txt:
	export ITERS=100000
	echo > $(REPORT_DIR)/$@
	./armemu sum_array 1 2 3 4 5 6 7 8 9 10 >> $(REPORT_DIR)/$@
	./armemu sum_array -1 -5 -18 -333333 >> $(REPORT_DIR)/$@
	./armemu sum_array 0 0 0 0 0 0 >> $(REPORT_DIR)/$@

fib_iter.txt:
	export ITERS=100000
	echo > $(REPORT_DIR)/$@
	./armemu fib_iter 10 >> $(REPORT_DIR)/$@
	./armemu fib_iter 19 >> $(REPORT_DIR)/$@

fib_rec.txt:
	export ITERS=10000
	echo > $(REPORT_DIR)/$@
	./armemu fib_rec 10 >> $(REPORT_DIR)/$@
	./armemu fib_rec 19 >> $(REPORT_DIR)/$@

find_str.txt:
	export ITERS=10000
	./armemu find_str "Les chaussettes de l'archi-duchesse sont-elles seches et archi-seches" "chaussettes" >> $(REPORT_DIR)/$@
	./armemu find_str "Les chaussettes de l'archi-duchesse sont-elles seches et archi-seches" "archibald" >> $(REPORT_DIR)/$@

clean:
	rm -rf $(OBJS) $(PROG) $(REPORT_DIR)/
