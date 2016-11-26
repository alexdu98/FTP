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

// Pour les threads
#include <pthread.h>
#include <sys/syscall.h>

#define LG_FILE_ATTENTE 1
#define PATH_TO_STORAGE_DIR "./files/"
#define BEGIN 0

struct msg {
	int cmd;
	int content_size;
	char content[256];
};


/***********
 *** MAIN
 ***********/
int main(int argc, char* argv[]) {

	int go = 1;
	char r_buffer[256];

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
  
  int ret_shutdown = 0, ret_send = 0, ret_recv = 0, ret_close = 0;
  int nb_clients = 0;
  
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
		struct msg m_send;
		m_send.cmd = BEGIN;
		m_send.content_size = 0;
		
		while (ret_send < sizeof(m_send)) {
			
			ret_send += send(fd_circuitV, &m_send + ret_send, sizeof(m_send) - ret_send, 0);
			if(ret_send == -1) {
				perror("send begin ");
			}
		}
		
	  // BOUCLE DE RECEPTION DES COMMANDES
		while(online) {
			ret_recv = recv(fd_circuitV, r_buffer, sizeof(r_buffer), 0);

			if(ret_recv == -1) {
				perror("recv cmd ");
				
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
			else {
				r_buffer[ret_recv] = '\0';
				
				char* cmd_buffer = strdup(r_buffer);
				if(cmd_buffer == NULL) perror("strdup ");

				char delim = ' ';
				char* token = strsep(&cmd_buffer, &delim);

				printf("Cmd received : %s\n", token);
				
				if(strcmp(token, "GETLIST") == 0) {
		/* int fd_storage_dir = open(PATH_TO_STORAGE_DIR, ) */
				}
				else {
					
				}
			}
			
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
