CC = gcc
CFLAGS = -c
LFLAGS = -lpthread
OBJS_CLIENT = client.o commun.o
OBJS_SERVER = serveur.o commun.o 

all: serveur client

.c.o: 
	$(CC) $(CFLAGS) $<

serveur: $(OBJS_SERVER) 
	$(CC) $(OBJS_SERVER) $(LFLAGS) -o $@ 

client: $(OBJS_CLIENT) 
	$(CC) $(OBJS_CLIENT) $(LFLAGS) -o $@ 

clean:
	rm main *.o *~;
