all: serveur client clean

serveur: serveur.o
	gcc -o serveur serveur.o

serveur.o: serveur.c
	gcc -o serveur.o -c serveur.c

client: client.o
	gcc -o client client.o

client.o: client.c
	gcc -o client.o -c client.c

clean:
	rm *.o