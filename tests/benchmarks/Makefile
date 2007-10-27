TABLE_APP=table
HASH_APP=hash

CC=c99
INCS=-I../../include
CFLAGS=-W -Wall -O2 $(INCS) -g `pkg-config --cflags glib-2.0`
LIBS= `pkg-config --libs glib-2.0` -lm
LDFLAGS=-lcuckoo -L../../src $(LIBS)

TABLE_OBJS=table.o
HASH_OBJS=hash.o

all: $(APP) tests

tests: table_test hash_test

table_test: $(TABLE_APP)
	rm -f ./fifo
	mkfifo ./fifo
	zcat ./data/*.gz ./data/*.gz ./data/*.gz ./data/*.gz > ./fifo &
	LD_LIBRARY_PATH="../../src" ./$(TABLE_APP) ./fifo

table_debug: $(TABLE_APP)
	rm -f ./fifo
	mkfifo fifo
	zcat ./data/*.gz > fifo &
	LD_LIBRARY_PATH="../../src" gdb ./$(APP)

$(TABLE_APP): $(TABLE_OBJS)
	$(CC) -o $(TABLE_APP) $(TABLE_OBJS) $(LDFLAGS)

hash_test: $(HASH_APP)
	rm -f ./fifo
	mkfifo ./fifo
	zcat ./data/*.gz ./data/*.gz ./data/*.gz ./data/*.gz > ./fifo &
	LD_LIBRARY_PATH="../../src" ./$(HASH_APP) ./fifo

hash_debug: $(HASH_APP)
	rm -f ./fifo
	mkfifo fifo
	zcat ./data/*.gz > fifo &
	LD_LIBRARY_PATH="../../src" gdb ./$(HASH_APP)

$(HASH_APP): $(HASH_OBJS)
	$(CC) -o $(HASH_APP) $(HASH_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TABLE_APP) $(TABLE_OBJS) $(HASH_APP) $(HASH_OBJS)
