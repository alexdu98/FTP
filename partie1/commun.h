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

#define SIZE_BUFFER_CONTENT_FILE 2048

// COMMANDS
#define BEGIN 0
#define GETLIST 1
#define GET 2

struct file_infos {
  int file_nb;
  char infos[256];
};

union content {
	char file_buffer[SIZE_BUFFER_CONTENT_FILE];
	char file_infos[256][256];
};

union infos_contenu {
	int nb_fichier;
	int taille_fichier;
};

struct msg {
  int msg_size; // taille du message entier
  int cmd; // identifiant de la commande à exécuter
  union infos_contenu infos_contenu;
  union content content;
};

char* lstattoa(char* path_to_file, char* name);
//struct f_list* listdir(char* path_to_dir);
void listdir(char* path_to_dir, struct msg *msg);