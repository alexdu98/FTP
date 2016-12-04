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


// ##############################
// ######### CONSTANTES #########
// ##############################

#define SIZE_BUFFER_CONTENT 2048

// COMMANDES
#define BEGIN 0
#define GETLIST 1
#define GET 2
#define SIZE 3
#define ERROR 4
#define CONTENT_FILE 5

// ##############################
// ######### STRUCTURES #########
// ##############################

struct msg {
  unsigned int size; // taille du message entier
  int cmd; // identifiant de la commande à exécuter
  char content[SIZE_BUFFER_CONTENT]; // contenu du message
};


// ##############################
// ######### FONCTIONS ##########
// ##############################

int msg_send(int fd_circuitV, struct msg* m_send, int onlyContent);
int msg_recv(int fd_circuitV, struct msg* m_recv, int onlyContent);