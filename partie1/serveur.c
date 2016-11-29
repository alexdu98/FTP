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
  int ret_m_send = 0;
  
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

      ret_m_send = msg_send(fd_circuitV, &m_send);
      if(ret_m_send == 0) { // Le client s'est deconnecte
	nb_clients--;
	online = 0;
      }

      
      // BOUCLE DE RECEPTION DES COMMANDES
      while(online) {

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

      	/* TRAITEMENT DE LA CMD ET RENVOI DU RESULTAT */
      	switch(m_recv.cmd) {
	case GETLIST :
	  printf("Cmd GETLIST\n");
            
	  //struct f_list* list = listdir(PATH_TO_STORAGE_DIR);
	  //if(list == NULL) perror("listdir serveur ");

	  m_send.cmd = GETLIST;
	  m_send.infos_contenu.nb_fichier = 0;
	  listdir(PATH_TO_STORAGE_DIR, &m_send);
	  
	  m_send.msg_size = sizeof(m_send.msg_size)
	    + sizeof(m_send.cmd)
	    + sizeof(m_send.infos_contenu)
	    + (sizeof(m_send.content.file_infos[0]) * m_send.infos_contenu.nb_fichier);

	  printf("sizeof(msg) : %d \n", m_send.msg_size);
	  printf("sizeof(m_send.content.file_infos[0]) : %lu \n", sizeof(m_send.content.file_infos[0]));
	  printf("m_send.infos_contenu.nb_fichier : %d \n", m_send.infos_contenu.nb_fichier);

	  printf("%p : adr msg \n", &m_send);
	  printf("%p : adr msg_size \n", &m_send.msg_size);
	  printf("%p : adr cmd \n", &m_send.cmd);
	  printf("%p : adr infos_contenu \n", &m_send.infos_contenu);
	  printf("%p : adr content \n", &m_send.content);
	  printf("%p : adr nb_fichier \n", &m_send.infos_contenu.nb_fichier);
	  printf("%p : adr taille_fichier \n", &m_send.infos_contenu.taille_fichier);
	  printf("%p : adr file_buffer \n", &m_send.content.file_buffer);
	  printf("%p : adr file_infos \n", &m_send.content.file_infos);
	  printf("%p : adr file_infos 0 \n", &m_send.content.file_infos[0]);
	  printf("%p : adr file_infos 00 \n", &m_send.content.file_infos[0][0]);
	  printf("%p : adr file_infos 01 \n", &m_send.content.file_infos[0][1]);
	  printf("%p : adr file_infos 1 \n", &m_send.content.file_infos[1]);

	  ret_m_send = msg_send(fd_circuitV, &m_send);
	  if(ret_m_send == 0) { // Le client s'est deconnecte
	    nb_clients--;
	    online = 0;
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

