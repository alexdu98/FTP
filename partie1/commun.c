#include "commun.h"

//struct f_list* listdir(char* path_to_dir) {
void listdir(char* path_to_dir, struct msg *msg) {
  DIR* dir;
  
  dir = opendir(path_to_dir);
  if(dir == NULL) {
    perror("opendir ");
    //return NULL;
  }

  char* file_list[256];
  
  char path_to_entry [256];
  int numFichier = 0;
  
  struct dirent* entry;
  char* entry_infos;
  char cat_buffer [256];
  
  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    strcpy(path_to_entry, path_to_dir);
    strcat(path_to_entry, "/");
    strcat(path_to_entry, entry->d_name);

    //content->file_buffer = "aa";
    msg->infos_contenu.nb_fichier = numFichier + 1;
    //content->file_list[numFichier].infos = lstattoa(path_to_entry, entry->d_name);
    strcpy(msg->content.file_infos[numFichier], lstattoa(path_to_entry, entry->d_name));

    /*if(entry_infos == NULL) {
      perror("lstattoa ");

      strcpy(cat_buffer, "\n");
      strcpy(entry->d_name, strcat(cat_buffer, entry->d_name));
    
          
      strcpy(cat_buffer, "No infos ");
      strcpy(entry_infos, strcat(cat_buffer, entry->d_name));
      } else strcpy(entry_infos, strcat(entry_infos, entry->d_name));*/

    
    //file_list[offset] = entry_infos;

    numFichier++;
  }

  strcpy(msg->content.file_infos[numFichier], "\0");

  //struct f_list* fl = malloc(sizeof(*fl));
  //fl->file_nb = numFichier;
  // fl->list = file_list;

  //free(fl);
  
  //return fl; 
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
  
  while (s_total_size < sizeof(m_send->msg_size)) {
        			
    ret_send = send(fd_circuitV, m_send + s_total_size, m_send->msg_size - s_total_size, 0);
    if(ret_send == -1) {
      perror("send msg_send ");
      return -1;
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

    s_total_size += ret_send
  }

  return s_total_size;

}


int msg_recv(int fd_circuitV, struct msg* m_recv) {

  // Reception des 4 premiers octets contenant la taille du msg
  while (r_total_size < sizeof(m_recv.msg_size)) {
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

}

