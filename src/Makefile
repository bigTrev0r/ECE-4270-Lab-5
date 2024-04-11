mu-riscv: mu-riscv.c riscv_utils.c print_inst.c
	gcc -Wall -g -O2 $^ -o $@ -lm

.PHONY: clean
clean:
	rm -rf *.o *~ mu-riscv
