CC = sm_20
NVCC	= nvcc -O4 -arch=$(CC)

VERSION=0.04

all:
	$(NVCC) --default-stream per-thread main.c data.c backpropagation_train.cu train_Pthread.c backpropagation_classify.c -o main 

clean:
	rm -f *~ *.o *.bin main 

release:
	tar -cvf main-code-$(VERSION).tar *.c *.h *m *sh Makefile
