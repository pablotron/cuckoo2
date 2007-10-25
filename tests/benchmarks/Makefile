APP=parse
CC=c99
INCS=-I../../include
CFLAGS=-W -Wall -O2 -g $(INCS)
LIBS=-lcuckoo -dl
LDFLAGS=-L../../src $(LIBS)
OBJS=parse.o

all: $(APP) tests

tests: $(APP)
	rm -f ./fifo
	mkfifo ./fifo
	zcat ./data/*.gz ./data/*.gz ./data/*.gz ./data/*.gz > ./fifo &
	LD_LIBRARY_PATH="../../src" ./$(APP) ./fifo

debug: $(APP)
	rm -f ./fifo
	mkfifo fifo
	zcat ./data/*.gz > fifo &
	LD_LIBRARY_PATH="../../src" gdb ./$(APP)

$(APP): $(OBJS)
	$(CC) -o $(APP) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(APP) $(OBJS)
