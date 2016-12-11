#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pwd.h>
#include <netdb.h>

#include <signal.h>
#include <ctype.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <pthread.h>
#include <sys/syscall.h>

#include <dirent.h>


// ##################################
// #########   CONSTANTES   #########
// ##################################

// Taille du buffer d'un message
#define SIZE_BUFFER_CONTENT 2048

// Identifiant des appelants pour les msg_send et msg_recv
#define SERVEUR 0
#define CLIENT 1

// Commandes disponibles pour un message
#define BEGIN 0
#define GETLIST 1
#define ACK_GETLIST 2
#define GET 3
#define SIZE 4
#define ERROR 5
#define ACK_SIZE 6
#define CONTENT_FILE 7
#define ACK_CONTENT_FILE 8


// ##################################
// #########   STRUCTURES   #########
// ##################################

struct msg {
  unsigned int size; // taille du message entier
  int cmd; // identifiant de la commande à exécuter
  char content[SIZE_BUFFER_CONTENT]; // contenu du message
};


// ##################################
// #########   FONCTIONS   ##########
// ##################################

int msg_send(int fd_circuitV, struct msg* m_send, int appelant);
int msg_recv(int fd_circuitV, struct msg* m_recv, int appelant);