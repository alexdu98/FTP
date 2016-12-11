#include "commun.h"


// ##################################
// #########   CONSTANTES   #########
// ##################################

// Longueur de la file d'attente
#define LG_FILE_ATTENTE 1


// ##################################
// #########   FONCTIONS   ##########
// ##################################

char* lstattoa(char* path_to_file, char* name);
int listdir(const char* path_to_dir, struct msg* msg, int socket);