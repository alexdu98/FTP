#include "commun.h"

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

/*
 * @PARAM fd_circuitV, le descripteur de la socket associer
 * a l'envoi de msg
 * @PARAM m_send, la struct a envoyer par message
 * 
 * @RETURN le nombre d'octets envoyes si tout se passe bien,
 * 0 si le pair se deconnecte, et -1 en cas d'erreur.
 */
int msg_send(int fd_circuitV, struct msg* m_send) {

  int ret_send = 0, ret_shutdown = 0, ret_close = 0;
  int s_total_size = 0;

  while (s_total_size < sizeof(m_send->size)) {		

    ret_send = send(fd_circuitV, m_send + s_total_size, m_send->size - s_total_size, 0);

    if(ret_send == -1) {
      perror("send msg_send ");
      return 0;
    }
    // Si le client se deconnecte intempestivement
    else if(ret_send == 0) {   
      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      if(ret_shutdown == -1) perror("msg_send shutdown ");

      // Ferme le descripteur de la socket
      ret_close = close(fd_circuitV);
      if(ret_close == -1) perror("msg_send close ");
      return 0;        			       
    }

    s_total_size += ret_send;
  }
  return s_total_size;

}


int msg_recv(int fd_circuitV, struct msg* m_recv) {

  int ret_recv = 0, ret_shutdown = 0, ret_close = 0;
  int r_total_size = 0;

  // Reception des 4 premiers octets contenant la taille du msg
  while (r_total_size < sizeof(m_recv->size)) {
    ret_recv = recv(fd_circuitV, m_recv + r_total_size, sizeof(m_recv) - r_total_size, 0);

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
        return 0;
    }
      	  
    r_total_size += ret_recv;
  }

  // Taille du msg recu, maintenant reception du reste
  while(r_total_size < m_recv->size) {
    printf("%d/%d \n", r_total_size, m_recv->size);
    ret_recv = recv(fd_circuitV, m_recv + r_total_size, sizeof(m_recv) - r_total_size, 0);

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

      return 0;
    }

    r_total_size += ret_recv;
  }

  return r_total_size;

}

