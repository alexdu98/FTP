#include "serveur.h"


int main(int argc, char * argv[]) {

  if(argc > 3 || argc < 2 || (atoi(argv[1]) != 0 && atoi(argv[1]) <= 1024)){
    printf("Usage : mainS port(0 = rand, > 1024) [repertoireDL] \n");
    return EXIT_FAILURE;
  }

  const char* PATH_TO_STORAGE_DIR;
  if(argc == 3){
    if(access(argv[2], F_OK) == 0){
      PATH_TO_STORAGE_DIR = argv[2];
      if(argv[2][strlen(argv[2]) - 1] != '/'){
        char nomTemp[256];
        strcpy(nomTemp, argv[2]);
        strcat(nomTemp, "/");
        PATH_TO_STORAGE_DIR = nomTemp;
      }
    }
    else{
      printf("'%s' \n", argv[2]);
      perror("Repertoire de telechargement non valide ");
      return EXIT_FAILURE;
    }
  }
  else if(argc == 2){
    PATH_TO_STORAGE_DIR = "../repDL/";
  }

  printf("Repertoire de telechargement : %s \n", PATH_TO_STORAGE_DIR);

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
  brPublique.sin_port = htons(atoi(argv[1])); // num de port choisi par le systeme si 0

  // Liaison de la socket et de la struct 
  int ret_bind = bind(fd_brPublique, (struct sockaddr * ) & brPublique, sizeof(brPublique));
  if (ret_bind == -1) {
    perror("bind ");
    exit(EXIT_FAILURE);
  }

  // Afficher le numero de port pour les clients
  socklen_t length = sizeof(brPublique);
  getsockname(fd_brPublique, (struct sockaddr * ) & brPublique, & length);
  printf("Adresse : %s / Port : %u \n", inet_ntoa(brPublique.sin_addr), ntohs(brPublique.sin_port));

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

    printf("\nEn attente d'un client...\n");
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

      ret_m_send = msg_send(fd_circuitV, &m_send, SERVEUR);
      if (ret_m_send == 0) { // Le client s'est deconnecte
        nb_clients--;
        online = 0;
      }

      // BOUCLE DE RECEPTION DES COMMANDES
      while (online) {

        printf("\n");

        int res_recv = msg_recv(fd_circuitV, &m_recv, SERVEUR);
        if(res_recv == 0){
          nb_clients--;
          online = 0;
          break;
        }

        //printf("RECV :\n");
        //printf("size : %d \n", m_recv.size);
        //printf("cmd : %d \n", m_recv.cmd);

        /* TRAITEMENT DE LA CMD ET RENVOI DU RESULTAT */
        switch (m_recv.cmd) {
        case GETLIST:
          printf("CMD : GETLIST \n");

          //struct f_list* list = listdir(PATH_TO_STORAGE_DIR);
          //if(list == NULL) perror("listdir serveur ");

          m_send.cmd = GETLIST;
          listdir(PATH_TO_STORAGE_DIR, m_send.content);

          m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);

          ret_m_send = msg_send(fd_circuitV, &m_send, SERVEUR);

          if (ret_m_send == 0) { // Le client s'est deconnecte
            nb_clients--;
            online = 0;
          }

          printf("GETLIST sent (%d/%d) \n", ret_m_send, m_send.size);
          break;

          case GET:
            printf("CMD : GET (%s) \n", m_recv.content);

            /* Fichier à renvoyer */
            FILE* fichier = NULL;

            char nomFichier[255];

            unsigned int tailleFichier;

            /* Réinitialisation de la variable */
            memset(nomFichier, 0, sizeof(nomFichier));

            strcpy(nomFichier, PATH_TO_STORAGE_DIR);
            strcat(nomFichier, m_recv.content);

            /* Si on ne peut pas accéder au fichier */
            if((fichier = fopen(nomFichier, "rb")) == NULL){
              m_send.cmd = ERROR;
              strcpy(m_send.content, m_recv.content);
              strcat(m_send.content, " -> ");
              strcat(m_send.content, strerror(errno));
              printf("ERROR ");
            }
            /* S'il n'y a pas d'erreur */
            else{
              m_send.cmd = SIZE;
              fseek(fichier, 0, SEEK_END);
              tailleFichier = ftell(fichier);
              sprintf(m_send.content, "%u", tailleFichier);
              printf("SIZE ");
            }

            m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);
            ret_m_send = msg_send(fd_circuitV, &m_send, SERVEUR);

            printf("sent (%d/%d) [%s] \n", ret_m_send, m_send.size, m_send.content);

            if(m_send.cmd == ERROR) continue;

            char nomF[255]; strcpy(nomF, m_recv.content);

            int res_recv = msg_recv(fd_circuitV, &m_recv, SERVEUR);
            if(m_recv.cmd != ACK_SIZE){
              printf("Erreur : cmd %d attendu, cmd %d recu \n", ACK_SIZE, m_recv.cmd);
              continue;
            }

            unsigned int nbCarEnvTotal = 0;
            unsigned int carFseek = 0;
            unsigned int test = 0;
            int onlyContent = 0;
            m_send.cmd = CONTENT_FILE;
            while(test < tailleFichier){

              fseek(fichier, carFseek, SEEK_SET);

              unsigned int resfr = fread(m_send.content, 1, sizeof(m_send.content), fichier);
              m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + resfr;

              /*if(onlyContent > 0){
                m_send.size = resfr;
              }
              else{
                carFseek -= sizeof(m_send.size) + sizeof(m_send.cmd);
              }*/

              // printf("#%c\n", m_send.content[m_send.size - 9]);
              // printf("#%c\n", m_send.content[m_send.size - 8]);
              // printf("#%c\n", m_send.content[m_send.size - 7]);
              // printf("#%c\n", m_send.content[m_send.size - 6]);
              // printf("#%c\n", m_send.content[m_send.size - 5]);
              // printf("#%c\n", m_send.content[m_send.size - 4]);
              // printf("#%c\n", m_send.content[m_send.size - 3]);
              // printf("#%c\n", m_send.content[m_send.size - 2]);
              // printf("#%c\n", m_send.content[m_send.size - 1]);
              // printf("%d \n", m_send.size);
              ret_m_send = msg_send(fd_circuitV, &m_send, SERVEUR);
              nbCarEnvTotal += ret_m_send;
              carFseek += ret_m_send - (sizeof(m_send.size) + sizeof(m_send.cmd));
              test += ret_m_send;

              // onlyContent = 1;
            }

            fclose(fichier);

            printf("Le fichier %s a bien ete transmis. \n", nomF);

          break;
        }

        r_total_size = 0;
      }

      printf("Un client vient de se deconnecter. FD = %d\n", fd_circuitV);
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