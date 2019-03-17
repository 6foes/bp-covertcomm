all: rx tx

rx: rx.c shared_fns.h
	gcc rx.c -o rx -lrt -masm=intel -ggdb -lpthread

tx: tx.c shared_fns.h
	gcc tx.c -o tx -lrt -masm=intel -ggdb -lpthread

clean:
	rm -f rx tx out.csv