CC	= clang
CFLAGS	= -c -g -Wall -O3
LDFLAGS	=
BUILD	= mmzklist_example

all:		$(BUILD)

$(BUILD):		mmzklist_example.o ../mmzklist.o

mmzklist_example.o:	../mmzklist.h
../mmzklist.o:		../mmzklist.h

run:
	make all
	./$(BUILD)

test:
	make all
	leaks --atExit -- ./$(BUILD)

clean:
	rm -f -rf $(wildcard *.o) $(wildcard *.a) $(BUILD) *.dSYM
	cd ../; rm -f -rf *.o *.a *.dSYM
