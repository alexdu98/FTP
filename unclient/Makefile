CC = gcc
CFLAGS = -c -std=gnu11 -Wall
LFLAGS = -Wall
OBJS_CLIENT = mainC.o client.o commun.o
OBJS_SERVER = mainS.o serveur.o commun.o 

all: mainS mainC clean

.c.o: 
	$(CC) $(CFLAGS) $<

mainS: $(OBJS_SERVER) 
	$(CC) $(OBJS_SERVER) $(LFLAGS) -o $@ 

mainC: $(OBJS_CLIENT) 
	$(CC) $(OBJS_CLIENT) $(LFLAGS) -o $@ 

clean:
	rm *.o