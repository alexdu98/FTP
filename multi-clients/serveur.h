#include "commun.h"


// ##################################
// #########   CONSTANTES   #########
// ##################################

// Longueur de la file d'attente
#define LG_FILE_ATTENTE 1
#define NB_MAX_CLIENTS 10

// ##################################
// #########   STRUCTURES   #########
// ##################################

struct shared_vars {
  int nb_clients;
  pthread_mutex_t lock;
};

struct client_args {
  int fd_circuitV;
  struct shared_vars* s_vars;
};

// ##################################
// #########   FONCTIONS   ##########
// ##################################

char* lstattoa(char* path_to_file, char* name);
void listdir(const char* path_to_dir, char* chaine);
