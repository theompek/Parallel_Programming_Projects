GCC=gcc -O4 -std=gnu99

VERSION=0.04

all:
	$(GCC) main.c data.c backpropagation_train.c  backpropagation_classify.c -lm -o main 

clean:
	rm -f *~ *.o *.bin main 

release:
	tar -cvf main-code-$(VERSION).tar *.c *.h *m *sh Makefile
