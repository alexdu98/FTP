#include "serveur.h"

/***********
 *** MAIN
 ***********/
int main(int argc, char* argv[]) {


  /**********************************************************************************
   ********************    MISE EN PLACE DE LA COMMUNICATION     ********************
   **********************************************************************************/
	
  // Creation de la boite publique
  int fd_brPublique = socket(PF_INET, SOCK_STREAM, 0);
  if(fd_brPublique == -1) {
    perror("socket ");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in brPublique;
  brPublique.sin_family = AF_INET;
  brPublique.sin_addr.s_addr = INADDR_ANY;
  brPublique.sin_port = 0; // num de port choisi par le systeme

  // Liaison de la socket et de la struct 
  int ret_bind = bind(fd_brPublique, (struct sockaddr *) &brPublique, sizeof(brPublique));
  if(ret_bind == -1) {
    perror("bind ");
    exit(EXIT_FAILURE);
  }

  // Afficher le numero de port pour les clients
  socklen_t length = sizeof(brPublique);
  getsockname(fd_brPublique, (struct sockaddr*) &brPublique, &length);

  printf("Le numéro de la boîte publique est %d \n", ntohs(brPublique.sin_port));

  // Creation d'une file d'attente des connexions
  int ret_listen = listen(fd_brPublique, LG_FILE_ATTENTE);
  if(ret_listen == -1) {
    perror("listen ");
    exit(EXIT_FAILURE);
  }

  /****************************************************************
   ***************    TRAITEMENT DU CLIENT     ********************
   ****************************************************************/
  
  int go = 1;
  int ret_shutdown = 0, ret_send = 0, ret_recv = 0, ret_close = 0;
  int r_total_size = 0, s_total_size = 0;
  int nb_clients = 0;

  struct msg m_send;
  struct msg m_recv;
  
  struct sockaddr_in brCv;
  socklen_t lgbrCv = sizeof(brCv);
  
  int fd_circuitV = 0;
  int online = 0;
  
  while(go) {

    // Acceptation d'une connexion de client
    fd_circuitV = accept(fd_brPublique, (struct sockaddr * ) &brCv, &lgbrCv);
	
    // Augmente le nb de clients apres acceptation
    nb_clients++;
	
    if(fd_circuitV == -1) {
      perror("accept ");
    }
    // Traitement normal du client
    else { 
      printf("Un client vient de se connecter. FD = %d\n", fd_circuitV);
      online = 1;
      
      // Envoi message au client pour commencer les transactions
      m_send.msg_size = sizeof(m_send.msg_size) + sizeof(m_send.cmd) + strlen(m_send.msg_content);
      m_send.cmd = BEGIN;
      
      // Tant que tout le msg n'est pas envoye, on boucle
      while (s_total_size < sizeof(m_send.msg_size)) {
			
      	ret_send = send(fd_circuitV, &m_send + s_total_size, m_send.msg_size - s_total_size, 0);
      	if(ret_send == -1) {
      	  perror("send begin ");
      	}
      	// Si le client se deconnecte intempestivement
      	else if(ret_send == 0) {   
      	  ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      	  if(ret_shutdown == -1) perror("shutdown disconnect ");

      	  // Ferme le descripteur de la socket
      	  ret_close = close(fd_circuitV);
      	  if(ret_close == -1) perror("close ");
      				
      	  nb_clients--;
      	  online = 0;
      	}

      	s_total_size += ret_send;
      }
      
      // BOUCLE DE RECEPTION DES COMMANDES
      while(online) {

      	// Reception des 4 premiers octets contenant la taille du msg
      	while (r_total_size < sizeof(m_send.msg_size)) {
      	  ret_recv = recv(fd_circuitV, &m_recv + r_total_size, sizeof(m_recv) - r_total_size, 0);

      	  if(ret_recv == -1) {
      	    perror("recv cmd ");
      	    exit(EXIT_FAILURE);
      	  }
      	  // Si le client se deconnecte intempestivement
      	  else if(ret_recv == 0) {   
      	    ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      	    if(ret_shutdown == -1) perror("shutdown disconnect ");

      	    // Ferme le descripteur de la socket
      	    ret_close = close(fd_circuitV);
      	    if(ret_close == -1) perror("close ");
      				
      	    nb_clients--;
      	    online = 0;
      	  }
      	  
      	  r_total_size += ret_recv;
      	}


      	// Taille du msg recu, maintenant reception du reste
      	while(r_total_size < m_recv.msg_size) {
      	  ret_recv = recv(fd_circuitV, &m_recv + r_total_size, sizeof(m_recv) - r_total_size, 0);

      	  if(ret_recv == -1) {
      	    perror("recv cmd ");
      	    exit(EXIT_FAILURE);
      	  }
      	  // Si le client se deconnecte intempestivement
      	  else if(ret_recv == 0) {   
      	    ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      	    if(ret_shutdown == -1) perror("shutdown disconnect ");

      	    // Ferme le descripteur de la socket
      	    ret_close = close(fd_circuitV);
      	    if(ret_close == -1) perror("close ");
      				
      	    nb_clients--;
      	    online = 0;
      	  }

      	  r_total_size += ret_recv;
      	}

      	/* TRAITEMENT DE LA CMD ET RENVOI DU RESULTAT */
      	switch(m_recv.cmd) {
        	case GETLIST :
        	  printf("Cmd GETLIST\n");
            
        	  //struct f_list* list = listdir(PATH_TO_STORAGE_DIR);
        	  //if(list == NULL) perror("listdir serveur ");

        	  m_send.cmd = GETLIST;
            listdir(PATH_TO_STORAGE_DIR, m_send.msg_content);
        	  //m_send.content.file_list = *list;
        	  m_send.msg_size = sizeof(m_send.msg_size) + sizeof(m_send.cmd) + strlen(m_send.msg_content);

        	  s_total_size = 0;
        	  while (s_total_size < sizeof(m_send.msg_size)) {
        			
        	    ret_send = send(fd_circuitV, &m_send + s_total_size, m_send.msg_size - s_total_size, 0);
        	    if(ret_send == -1) {
        	      perror("send begin ");
        	    }
        	    // Si le client se deconnecte intempestivement
        	    else if(ret_send == 0) {   
        	      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
        	      if(ret_shutdown == -1) perror("shutdown disconnect ");

        	      // Ferme le descripteur de la socket
        	      ret_close = close(fd_circuitV);
        	      if(ret_close == -1) perror("close ");
        				
        	      nb_clients--;
        	      online = 0;
        	    }

        	    s_total_size += ret_send;
        	  }
        	  printf("file list sent\n");
        	  break;

        	case GET :
        	  break;
      	}
	
        r_total_size = 0;
      }
		
    }
	
  }
  
  // Ferme la communication entre sockets
  ret_shutdown = shutdown(fd_brPublique, SHUT_RDWR);
  if(ret_shutdown == -1) perror("shutdown ");

  // Ferme le descripteur de la socket
  ret_close = close(fd_brPublique);
  if(ret_close == -1) perror("close ");

  
  return EXIT_SUCCESS;
}

void listdir(char* path_to_dir, char* chaine){

  DIR* dir;
  dir = opendir(path_to_dir);
  if(dir == NULL) {
    perror("opendir ");
  }
  
  char path_file[512];
  int chaine_len = 0;
  struct dirent* entry;

  strcpy(chaine, "");

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    strcpy(path_file, path_to_dir);
    strcat(path_file, "/");
    strcat(path_file, entry->d_name);

    strcat(chaine, lstattoa(path_file, entry->d_name));

    chaine_len = strlen(chaine);
  }

}

char* lstattoa(char* path_to_file, char* name){
  
  struct stat file;
  char* file_infos = malloc(256 * sizeof(*file_infos));

  if(lstat(path_to_file, &file) < 0){
    perror("lstat ");
    return NULL;
  }

  // File type (d, - ou l)
  if(S_ISREG(file.st_mode)){
    strcpy(file_infos, "-");
  }
  if(S_ISDIR(file.st_mode)){
    strcpy(file_infos, "d");
  }
  if(S_ISLNK(file.st_mode)){
    strcpy(file_infos, "l");
  }
  
  // USR permissions tests
  if((file.st_mode & S_IRUSR) == S_IRUSR){
    strcpy(file_infos, strcat(file_infos, "r"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWUSR) == S_IWUSR){
    strcpy(file_infos, strcat(file_infos, "w"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXUSR) == S_IXUSR){
    strcpy(file_infos, strcat(file_infos, "x"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  // GRP permissions tests
  if((file.st_mode & S_IRGRP) == S_IRGRP){
    strcpy(file_infos, strcat(file_infos, "r"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWGRP) == S_IWGRP){
    strcpy(file_infos, strcat(file_infos, "w"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXGRP) == S_IXGRP){
    strcpy(file_infos, strcat(file_infos, "x"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  // OTH permissions tests
  if((file.st_mode & S_IROTH) == S_IROTH){
    strcpy(file_infos, strcat(file_infos, "r"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IWOTH) == S_IWOTH){
    strcpy(file_infos, strcat(file_infos, "w"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  if((file.st_mode & S_IXOTH) == S_IXOTH){
    strcpy(file_infos, strcat(file_infos, "x"));
  }
  else strcpy(file_infos, strcat(file_infos, "-"));

  // add space
  strcpy(file_infos, strcat(file_infos, " "));

  char file_size [10];
  int ret_sprintf = 0;

  ret_sprintf = sprintf(file_size, "%d", (int)file.st_size);
  if(ret_sprintf == -1) perror("sprintf ");
  else strcpy(file_infos, strcat(file_infos, file_size));

   // add space
  strcpy(file_infos, strcat(file_infos, "\t"));

  // add name
  strcpy(file_infos, strcat(file_infos, name));

  // add name
  strcpy(file_infos, strcat(file_infos, "\n"));
  
  return file_infos;
}
