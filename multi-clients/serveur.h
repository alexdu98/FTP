#include "commun.h"


// ##################################
// #########   CONSTANTES   #########
// ##################################

// Longueur de la file d'attente
#define LG_FILE_ATTENTE 1
#define NB_MAX_CLIENTS 3

// ###################################
// #########   STRUCTURES   ##########
// ###################################

struct shared_vars {
  char* path_to_storage_dir;
  int nb_clients;
  pthread_mutex_t lock;
  int file_places_libres [NB_MAX_CLIENTS];
  int fin_de_file;
  struct cpt_args *cpt;
};

struct client_args {
  int id_client;
  int fd_circuitV;
  struct shared_vars* s_vars;
};

struct compteur_dl {
  char fichier [256];
  unsigned int nbDl;
};

struct cpt_args {
  struct compteur_dl* cpt;
  unsigned int * nbFiles;
  pthread_mutex_t lock_cpt;
};

// ##################################
// #########   FONCTIONS   ##########
// ##################################

char* lstattoa(char* path_to_file, char* name);
int listdir(const char* path_to_dir, struct msg* msg, int socket);
void* thread_client (void* args);
int pop_file (int* file, int fin_de_file);
void* thread_console_serveur(void *args);
unsigned int countFiles(const char* path_to_dir);
void setCpt(struct compteur_dl* cpt, const char* path_to_dir);
void addCpt(struct compteur_dl* cpt, char* file, unsigned int *nbFiles);
void getDl(struct compteur_dl* cpt, unsigned int nbFiles);
