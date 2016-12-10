#include "commun.h"


// ##################################
// #########   CONSTANTES   #########
// ##################################

// Longueur de la file d'attente
#define LG_FILE_ATTENTE 1
#define NB_MAX_CLIENTS 10

// ###################################
// #########   STRUCTURES   ##########
// ###################################

struct shared_vars {
  char* path_to_storage_dir;
  int nb_clients;
  pthread_mutex_t lock;
  int file_places_libres [NB_MAX_CLIENTS];
  int fin_de_file;
};

struct client_args {
  int id_client;
  int fd_circuitV;
  struct shared_vars* s_vars;
};

// ##################################
// #########   FONCTIONS   ##########
// ##################################

char* lstattoa(char* path_to_file, char* name);
void listdir(const char* path_to_dir, char* chaine);
void* thread_client (void* args);
int pop_file (int* file, int fin_de_file);
