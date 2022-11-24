all: psearch1 psearch2 psearch3 psearch3slave psearch4 psearch4slave

psearch1: psearch1.o
	gcc psearch1.o -o psearch1 

psearch2: psearch2.o
	gcc psearch2.o -o psearch2 

psearch3: psearch3.o
	gcc psearch3.o -o psearch3 

psearch3slave: psearch3slave.o
	gcc psearch3slave.o -o psearch3slave 

psearch4: psearch4.o
	gcc psearch4.o -o psearch4 

psearch4slave: psearch4slave.o
	gcc psearch4slave.o -o psearch4slave

psearch1.o: psearch1.c
	gcc -c psearch1.c

psearch2.o: psearch2.c
	gcc -c psearch2.c

psearch3.o: psearch3.c
	gcc -c psearch3.c

psearch4.o: psearch4.c
	gcc -c psearch4.c

clean:
	rm *.o psearch1 psearch2 psearch3 psearch3slave