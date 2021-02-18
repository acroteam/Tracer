all: final.out

final.out: example.o syscall_handle.o tracer_final.o set.o
	gcc example.o syscall_handle.o tracer_final.o set.o -o final.out

example.o: example.c
	gcc -c example.c -o example.o

syscall_handle.o: syscall_handle.c
	gcc -c syscall_handle.c -o syscall_handle.o

tracer_final.o: tracer_final.c 
	gcc -c tracer_final.c -o tracer_final.o

set.o: set.c
	gcc -c set.c -o set.o