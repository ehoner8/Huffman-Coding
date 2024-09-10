CC = clang
CFLAGS = -Werror -Wall -Wextra -Wconversion -Wdouble-promotion -Wstrict-prototypes -pedantic
LFLAGS =

all: bwtest brtest nodetest pqtest huff dehuff

bwtest: bwtest.o bitwriter.o
	$(CC) $^ $(LFLAGS) -o $@

brtest: brtest.o bitreader.o
	$(CC) $^ $(LFLAGS) -o $@

nodetest: nodetest.o node.o
	$(CC) $^ $(LFLAGS) -o $@

pqtest: pqtest.o pq.o node.o
	$(CC) $^ $(LFLAGS) -o $@

huff: huff.o bitwriter.o node.o pq.o
	$(CC) $^ $(LFLAGS) -o $@

dehuff: dehuff.o bitreader.o node.o pq.o
	$(CC) $^ $(LFLAGS) -o $@

#calc: calc.o stack.o mathlib.o operators.o
        #$(CC) $^ $(LFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o bwtest brtest nodetest pqtest huff dehuff

format:
	clang-format -i -style=file *.[ch]

.PHONY: all clean format
