CC	= clang
CFLAGS	= -c -g -Wall -I$(HOME)/c-tools/include/ -O3
LDFLAGS	= -L$(HOME)/c-tools/lib/ -lmmzktestbase
BUILD	= mmzklist_test

all:		$(BUILD)

$(BUILD):		mmzklist_test.o ../mmzklist.o

mmzklist_test.o:	../mmzklist.h ../mmzklist_base.h
../mmzklist.o:		../mmzklist.h ../mmzklist_base.h

run:
	make all
	./$(BUILD)

test:
	make all
	leaks --atExit -- ./$(BUILD)

clean:
	rm -f -rf $(wildcard *.o) $(wildcard *.a) $(BUILD) *.dSYM
	cd ../; rm -f -rf *.o *.a *.dSYM
