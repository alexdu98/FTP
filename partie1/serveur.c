#include "serveur.h"

/***********
 *** MAIN
 ***********/
int main(int argc, char * argv[]) {

  /**********************************************************************************
   ********************    MISE EN PLACE DE LA COMMUNICATION     ********************
   **********************************************************************************/

  // Creation de la boite publique
  int fd_brPublique = socket(PF_INET, SOCK_STREAM, 0);
  if (fd_brPublique == -1) {
    perror("socket ");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in brPublique;
  brPublique.sin_family = AF_INET;
  brPublique.sin_addr.s_addr = INADDR_ANY;
  brPublique.sin_port = 0; // num de port choisi par le systeme

  // Liaison de la socket et de la struct 
  int ret_bind = bind(fd_brPublique, (struct sockaddr * ) & brPublique, sizeof(brPublique));
  if (ret_bind == -1) {
    perror("bind ");
    exit(EXIT_FAILURE);
  }

  // Afficher le numero de port pour les clients
  socklen_t length = sizeof(brPublique);
  getsockname(fd_brPublique, (struct sockaddr * ) & brPublique, & length);

  printf("Le numéro de la boîte publique est %d \n", ntohs(brPublique.sin_port));

  // Creation d'une file d'attente des connexions
  int ret_listen = listen(fd_brPublique, LG_FILE_ATTENTE);
  if (ret_listen == -1) {
    perror("listen ");
    exit(EXIT_FAILURE);
  }

  /****************************************************************
   ***************    TRAITEMENT DU CLIENT     ********************
   ****************************************************************/

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

  while(1) {

    // Acceptation d'une connexion de client
    fd_circuitV = accept(fd_brPublique, (struct sockaddr * ) & brCv, & lgbrCv);

    // Augmente le nb de clients apres acceptation
    nb_clients++;

    if (fd_circuitV == -1) {
      perror("accept ");
    }
    else { // Traitement normal du client
      printf("Un client vient de se connecter. FD = %d\n", fd_circuitV);
      online = 1;

      // Envoi message au client pour commencer les transactions
      m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);
      m_send.cmd = BEGIN;

      ret_m_send = msg_send(fd_circuitV, &m_send);
      if (ret_m_send == 0) { // Le client s'est deconnecte
        nb_clients--;
        online = 0;
      }

      // BOUCLE DE RECEPTION DES COMMANDES
      while (online) {

        int res_recv = msg_recv(fd_circuitV, &m_recv);
        if(res_recv == 0){
          nb_clients--;
          online = 0;
        }

        /* TRAITEMENT DE LA CMD ET RENVOI DU RESULTAT */
        switch (m_recv.cmd) {
        case GETLIST:
          printf("CMD : GETLIST\n");

          //struct f_list* list = listdir(PATH_TO_STORAGE_DIR);
          //if(list == NULL) perror("listdir serveur ");

          m_send.cmd = GETLIST;
          listdir(PATH_TO_STORAGE_DIR, m_send.content);

          printf("%s \n", m_send.content);

          m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);
          printf("%d !\n", m_send.size);
          ret_m_send = msg_send(fd_circuitV, &m_send);
          printf("b\n");
          if (ret_m_send == 0) { // Le client s'est deconnecte
            nb_clients--;
            online = 0;
          }

          printf("GETLIST sent \n");
          break;

        case GET:
          break;
        }

        r_total_size = 0;
      }
    }
  }

  // Ferme la communication entre sockets
  ret_shutdown = shutdown(fd_brPublique, SHUT_RDWR);
  if (ret_shutdown == -1) perror("shutdown ");

  // Ferme le descripteur de la socket
  ret_close = close(fd_brPublique);
  if (ret_close == -1) perror("close ");

  return EXIT_SUCCESS;
}