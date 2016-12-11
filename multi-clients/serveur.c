#include "serveur.h"

/*
 * @PARAM file, un tableau d'entiers faisant office de file.
 * 
 * @PARAM fin_de_file, l'indice correspondant au 
 * dernier element de la file
 *
 * @RETURN retourne l'element situe a la position zero dans la file
 * et fait avancer la file, c'est a dire decale tous les elements 
 * a gauche depuis l'indice 1 jusqu'a l'indice fin_de_file
 */
int pop_file (int* file, int fin_de_file) {
  int first = file[0];

  for(int i = 1; i <= fin_de_file; i++) 
    file[i - 1] = file[i];

  return first;
  
}

/*********************************
 *********************************
 *    FONCTION THREAD SERVEUR    *
 *********************************
 *********************************/
void* thread_console_serveur(void *args){

  char cmd[256];
  struct cpt_args* my_props = (struct cpt_args*) args;

  while(1){
    memset(cmd, 0, sizeof(cmd));

    printf("\n\n> Appuyez sur la touche entree pour voir le compteur de telechargement...\n\n");

    fgets(cmd, sizeof(cmd), stdin);

    // Il ne faut pas que les threads essayent de lire/écrire en même temps sur le nombre de téléchargement d'un fichier
    int ret_m_lock = pthread_mutex_lock(&(my_props->lock_cpt));
    if(ret_m_lock != 0)
      perror("mutex lock nb clients inc ");

    // Affiche le nombre de téléchargement par fichier
    getDl(my_props->cpt, *my_props->nbFiles);

    int ret_m_unlock = pthread_mutex_unlock(&(my_props->lock_cpt));
    if(ret_m_unlock != 0)
      perror("mutex unlock nb clients inc ");
  }

  pthread_exit(NULL);
}

/*********************************
 *********************************
 *     FONCTION THREAD CLIENT    *
 *********************************
 *********************************/
void* thread_client (void* args) {

  struct client_args* my_props = (struct client_args*) args;
  struct shared_vars* s_vars = my_props->s_vars;
  
  // Message reçu et envoye
  struct msg m_send;
  struct msg m_recv;

  // Retour du send
  int resSend;

  // Fichier rendu par fopen
  FILE* fichier;

  // Chemin et nom du fichier à ouvrir
  char cheminFichier[255];
  char nomFichier[255];

  // Taille du fichier à envoyer
  unsigned int tailleFichier;

  // Offset pour le fseek
  unsigned int offsetFseek;

  // Résultat du fread, nombre de caractères lu
  unsigned int resfr;

  // Client connecté = 1, déconnecté = 0
  int connecte;

  printf("Un client vient de se connecter. (socket : %d) \n", my_props->fd_circuitV);
  connecte = 1;

  // Message pour autoriser les échanges, pas de contenu
  m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd);
  m_send.cmd = BEGIN;

  // Si le client s'est déconnecté
  if(msg_send(my_props->fd_circuitV, &m_send, SERVEUR) == 0) connecte = 0;
    
  // #########################################
  // ##########   BOUCLE RECEP MSG   #########
  // #########################################

  while(connecte) {

    // Si le client s'est déconnecté
    if(msg_recv(my_props->fd_circuitV, &m_recv, SERVEUR) == 0) break;

    // #########################################
    // ############   CMD GETLIST   ############
    // #########################################
    if(m_recv.cmd == GETLIST){
        
      printf("CMD : GETLIST (socket : %d) \n", my_props->fd_circuitV);

      m_send.cmd = GETLIST;
      // Récupération des informations des fichiers du répertoire de téléchargement
      if(listdir(s_vars->path_to_storage_dir, &m_send, my_props->fd_circuitV) == 0) break;

      m_send.cmd = ACK_GETLIST;
      m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd);

      // Si le client s'est déconnecté
      if(msg_send(my_props->fd_circuitV, &m_send, SERVEUR) == 0) break;

      printf("GETLIST sent (socket : %d) \n", my_props->fd_circuitV);
    }
    // #########################################
    // ##############   CMD GET   ##############
    // #########################################
    else if(m_recv.cmd == GET){

      printf("CMD : GET (%s) (socket : %d) \n", m_recv.content, my_props->fd_circuitV);

      // Copie le nom du fichier
      strcpy(nomFichier, m_recv.content);

      // Concaténation du chemin du répertoire de téléchargement et du nom du fichier
      strcpy(cheminFichier, s_vars->path_to_storage_dir);
      strcat(cheminFichier, m_recv.content);

      fichier = NULL;

      // Si on ne peut pas accéder au fichier on envoit l'erreur
      if((fichier = fopen(cheminFichier, "rb")) == NULL){
      	m_send.cmd = ERROR;
      	strcpy(m_send.content, m_recv.content);
      	strcat(m_send.content, " -> ");
      	strcat(m_send.content, strerror(errno));
      }
      // S'il n'y a pas d'erreur on envoit la taille du fichier
      else{
      	m_send.cmd = SIZE;
      	fseek(fichier, 0, SEEK_END);
      	tailleFichier = ftell(fichier);
      	sprintf(m_send.content, "%u", tailleFichier);
      }

      m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);

      // Si le client s'est déconnecté
      if(msg_send(my_props->fd_circuitV, &m_send, SERVEUR) == 0) break;

      // Affiche la taille du fichier, ou l'erreur
      printf("Contenu envoye (size ou erreur) (socket : %d) : %s \n", my_props->fd_circuitV, m_send.content);

      // S'il y a eu une erreur on passe à la commande suivante
      if(m_send.cmd == ERROR) continue;

      // Si le client s'est déconnecté
      if(msg_recv(my_props->fd_circuitV, &m_recv, SERVEUR) == 0) break;

      // Si la commande n'est pas l'accusé de récéption de la taille du fichier
      if(m_recv.cmd != ACK_SIZE){
      	printf("Erreur : cmd %d attendu, cmd %d recu (socket : %d) \n", ACK_SIZE, m_recv.cmd, my_props->fd_circuitV);
      	continue;
      }

      m_send.cmd = CONTENT_FILE;

      offsetFseek = 0;
      while(offsetFseek < tailleFichier){

      	// Place le curseur pour la prochaine lecture
      	if(fseek(fichier, offsetFseek, SEEK_SET) == -1){
      	  perror("Erreur fseek ");
      	  break;
      	}

      	// Lecture du fichier
      	resfr = fread(m_send.content, 1, sizeof(m_send.content), fichier);

      	// On envoit la taille que l'on a lu
      	m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + resfr;

      	resSend = msg_send(my_props->fd_circuitV, &m_send, SERVEUR);

      	// Si le client s'est déconnecté
      	if(resSend == 0){
      	  connecte = 0;
      	  break;
      	}

      	// Decale l'offset
      	offsetFseek += resSend - (sizeof(m_send.size) + sizeof(m_send.cmd));
      }

      if(fclose(fichier) == EOF){
      	perror("Erreur fclose ");
      	break;
      }

      // Si le client s'était déconnecté
      if(connecte == 0) break;

      // Si le client s'est déconnecté
      if(msg_recv(my_props->fd_circuitV, &m_recv, SERVEUR) == 0) break;

      // Si la commande n'est pas l'accusé de récéption du fichier
      if(m_recv.cmd != ACK_CONTENT_FILE){
      	printf("Erreur : cmd %d attendu, cmd %d recu (socket : %d) \n", ACK_CONTENT_FILE, m_recv.cmd, my_props->fd_circuitV);
      	continue;
      }

      // Il ne faut pas que les threads essayent de lire/écrire en même temps sur le nombre de téléchargement d'un fichier
      pthread_mutex_lock(&((s_vars->cpt)->lock_cpt));

      addCpt((s_vars->cpt)->cpt, nomFichier, (s_vars->cpt)->nbFiles);

      pthread_mutex_unlock(&((s_vars->cpt)->lock_cpt));

      printf("Le fichier %s a bien ete transmis. (socket : %d)\n", nomFichier, my_props->fd_circuitV);

    }
    // #########################################
    // ############   CMD INCONNUE   ###########
    // #########################################
    else{

      printf("Commande inconnue (socket : %d) \n", my_props->fd_circuitV);

    }
  }

  // Operation en ecriture sur la var partagee nb de clients, var a proteger
  int ret_m_lock = pthread_mutex_lock(&(s_vars->lock));
  if(ret_m_lock != 0)
    perror("mutex lock nb clients inc ");

  // Diminue le nb de clients apres deconnexion
  s_vars->nb_clients--;

  s_vars->fin_de_file++;

  s_vars->file_places_libres[s_vars->fin_de_file] = my_props->id_client;

  printf("Un client vient de se deconnecter. (socket : %d) \n", my_props->fd_circuitV);

  int ret_m_unlock = pthread_mutex_unlock(&(s_vars->lock));
  if(ret_m_unlock != 0)
    perror("mutex unlock nb clients inc ");

  pthread_exit(NULL);
}

// Compte le nombre de fichier du répertoire path_to_dir
unsigned int countFiles(const char* path_to_dir){

  unsigned int nb = 0;
  DIR* dir;
  struct dirent* entry;

  dir = opendir(path_to_dir);
  if(dir == NULL) {
    perror("opendir listdir ");
  }

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    nb++;
  }

  return nb;
}

// Initialise le nombre de téléchargement à 0 pour tous les fichiers
void setCpt(struct compteur_dl* cpt, const char* path_to_dir){

  int i = 0;
  DIR* dir;
  struct dirent* entry;

  dir = opendir(path_to_dir);
  if(dir == NULL) {
    perror("opendir listdir ");
  }

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    strcpy(cpt[i].fichier, entry->d_name);
    cpt[i].nbDl = 0;
    i++;
  }
}

// Ajoute un téléchargement pour le fichier file, s'il non présent dans le tableau, ajout
void addCpt(struct compteur_dl* cpt, char* file, unsigned int *nbFiles){
  for (int i = 0; i < *nbFiles; ++i){
    if(strcmp(cpt[i].fichier, file) == 0){
      cpt[i].nbDl++;
      return;
    }
  }

  // Le fichier n'était pas encore dans la liste (ajouté après le lancement du serveur)
  struct compteur_dl* temp = (struct compteur_dl*)realloc(cpt, sizeof(struct compteur_dl) * (*nbFiles + 1));

  // Remplace le pointeur de la scruture
  *cpt = *temp;

  // Ajoute le nom du fichier et le nombre de téléchargement
  strcpy(cpt[*nbFiles].fichier, file);
  cpt[*nbFiles].nbDl = 1;

  // Incrémente le nombre de fichier
  (*nbFiles)++;
}

// Affiche le nombre de téléchargement pour chaque fichier
void getDl(struct compteur_dl* cpt, unsigned int nbFiles){
  printf("\nNombre de telechargement par fichier : \n");
  printf("-------------------------------\n\n");
  for (int i = 0; i < nbFiles; ++i){
    printf("%s \t", cpt[i].fichier);

    if(strlen(cpt[i].fichier) < 7)
      printf("\t");

    printf(" : %d \n",cpt[i].nbDl);
  }
}

// Envoit les informations des fichiers du répertoire path_to_dir
int listdir(const char* path_to_dir, struct msg* msg, int socket){

  DIR* dir;
  dir = opendir(path_to_dir);
  if(dir == NULL) {
    perror("opendir listdir ");
  }
  
  char path_file[512];
  struct dirent* entry;
  char temp[512];
  msg->content[0] = '\0';

  while((entry = readdir(dir)) != NULL) {

    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    strcpy(path_file, path_to_dir);
    strcat(path_file, "/");
    strcat(path_file, entry->d_name);

    strcpy(temp, lstattoa(path_file, entry->d_name));

    // Si on peut pas rajouter le fichier actuel car dépassement, on envoit d'abord
    if(strlen(temp) + strlen(msg->content) >= sizeof(msg->content)){
      msg->size = sizeof(msg->size) + sizeof(msg->cmd) + strlen(msg->content);
      // Si le client s'est déconnecté
      if(msg_send(socket, msg, SERVEUR) == 0) return 0;

      // On écrase les anciennes informations
      strcpy(msg->content, temp);
    }
    else{
      // On concate dans le buffer du message
      strcat(msg->content, temp);
    }
  }

  // Si il y a des informations a envoyer
  if(strlen(msg->content) > 0){
    msg->size = sizeof(msg->size) + sizeof(msg->cmd) + strlen(msg->content);
    // Si le client s'est déconnecté
    if(msg_send(socket, msg, SERVEUR) == 0) return 0;
  }

  return 1;
}


// Retourne les informations du fichier name du répertoire path_to_file
char* lstattoa(char* path_to_file, char* name){
	
  struct stat file;
  char* file_infos = malloc(256 * sizeof(*file_infos));
  char file_size [10];
  int ret_sprintf = 0;

  if(lstat(path_to_file, &file) < 0){
    perror("lstat lstattoa ");
    return NULL;
  }

  // Type du fichier (d, - ou l)
  if(S_ISREG(file.st_mode)) strcpy(file_infos, "-");
  else if(S_ISDIR(file.st_mode)) strcpy(file_infos, "d");
  else if(S_ISLNK(file.st_mode)) strcpy(file_infos, "l");
	

  // Permissions du fichier pour l'utilisateur
  if((file.st_mode & S_IRUSR) == S_IRUSR) strcpy(file_infos, strcat(file_infos, "r"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWUSR) == S_IWUSR) strcpy(file_infos, strcat(file_infos, "w"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXUSR) == S_IXUSR) strcpy(file_infos, strcat(file_infos, "x"));
  else strcpy(file_infos, strcat(file_infos, "-"));


  // Permissions du fichier pour le groupe
  if((file.st_mode & S_IRGRP) == S_IRGRP) strcpy(file_infos, strcat(file_infos, "r"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWGRP) == S_IWGRP) strcpy(file_infos, strcat(file_infos, "w"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXGRP) == S_IXGRP) strcpy(file_infos, strcat(file_infos, "x"));
  else strcpy(file_infos, strcat(file_infos, "-"));


  // Permissions du fichier pour le monde
  if((file.st_mode & S_IROTH) == S_IROTH) strcpy(file_infos, strcat(file_infos, "r"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWOTH) == S_IWOTH) strcpy(file_infos, strcat(file_infos, "w"));
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXOTH) == S_IXOTH) strcpy(file_infos, strcat(file_infos, "x"));
  else strcpy(file_infos, strcat(file_infos, "-"));


  strcpy(file_infos, strcat(file_infos, " "));

  ret_sprintf = sprintf(file_size, "%d", (int)file.st_size);
  if(ret_sprintf == -1) perror("sprintf lstattoa ");
  else strcpy(file_infos, strcat(file_infos, file_size));

  if((int)file.st_size < 10000)
    strcpy(file_infos, strcat(file_infos, "\t"));

  strcpy(file_infos, strcat(file_infos, "\t"));
  strcpy(file_infos, strcat(file_infos, name));
  strcpy(file_infos, strcat(file_infos, "\n"));
	
  return file_infos;
}
