APP=cuckoo_test
INCS=-I../include
CFLAGS=-W -Wall -O2 -g $(INCS)
LIBS=-lcuckoo -dl
LDFLAGS=-L../src $(LIBS)
OBJS=main.o $(shell ls test_*.c | sed 's/\.c/.o/')

all: $(APP) tests

tests: $(APP)
	LD_LIBRARY_PATH="../src" ./$(APP)

$(APP): $(OBJS)
	$(CC) -o $(APP) $(OBJS) $(LDFLAGS)

main.o: main.c all_tests.txt
	./build_test_list.rb && $(CC) $(CFLAGS) -c main.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(APP) $(OBJS) $(TESTS)
