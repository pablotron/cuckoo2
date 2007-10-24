CC=cc
CFLAGS=-W -Wall -pedantic -g  -I../include
LDFLAGS=
LIBS=
LIB=libcuckoo.so
OBJS=hash_opt.o hash.o

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(OBJS) $(LIB)
  

