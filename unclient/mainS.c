#include "serveur.h"

int main(int argc, char * argv[]) {

  if(argc > 3 || argc < 2 || (atoi(argv[1]) != 0 && atoi(argv[1]) <= 1024)){
    printf("Usage : mainS port(0 = rand, > 1024) [repertoireDL] \n");
    return EXIT_FAILURE;
  }

  // #########################################
  // #####   DECLARATION DES VARIABLES   #####
  // #########################################

  // Repertoire contenant les fichiers pouvant être téléchargés
  const char* PATH_TO_STORAGE_DIR;

  // Descripteur de la socket publique
  int fd_brPublique;

  // Configuration de la socket publique et virtuelle
  struct sockaddr_in brPublique;
  struct sockaddr_in brCv;

  // Taille des scrutures sockaddr_in
  socklen_t lgbrCv = sizeof(brCv);
  socklen_t length;

  // Nombre de clients connectés
  int nb_clients = 0;

  // Message reçu et envoyé
  struct msg m_send;
  struct msg m_recv;

  // Descripteur de la socket virtuelle
  int fd_circuitV;

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

  // #########################################
  // ###   REPERTOIRE DE TELECHARGEMENT   ####
  // #########################################
  
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

  // #########################################
  // ########   INIT DE LA CONNEXION   #######
  // #########################################

  // Création de la boite publique
  fd_brPublique = socket(PF_INET, SOCK_STREAM, 0);
  if (fd_brPublique == -1) {
    perror("Erreur socket ");
  }

  // Configuration de la socket
  brPublique.sin_family = AF_INET;
  brPublique.sin_addr.s_addr = INADDR_ANY;
  brPublique.sin_port = htons(atoi(argv[1])); // num de port choisi par le systeme si 0

  // Liaison de la socket et de la structure
  if(bind(fd_brPublique, (struct sockaddr * ) & brPublique, sizeof(brPublique)) == -1){
    perror("Erreur bind ");
  }

  // Affiche le numéro de port pour les clients
  length = sizeof(brPublique);
  getsockname(fd_brPublique, (struct sockaddr * ) & brPublique, & length);
  printf("Adresse : %s / Port : %u \n", inet_ntoa(brPublique.sin_addr), ntohs(brPublique.sin_port));

  // Création d'une file d'attente des connexions
  if(listen(fd_brPublique, LG_FILE_ATTENTE) == -1){
    perror("Erreur listen ");
  }

  // #########################################
  // ##########   BOUCLE PRINCIPALE   ########
  // #########################################

  while(1) {

    printf("\nEn attente d'un client...\n");

    // Acceptation d'un client
    fd_circuitV = accept(fd_brPublique, (struct sockaddr * ) & brCv, & lgbrCv);

    if (fd_circuitV == -1) {
      perror("Erreur accept ");
      continue;
    }

    // Augmente le nombre de clients
    nb_clients++;

    printf("Un client vient de se connecter. (socket : %d) \n", fd_circuitV);
    connecte = 1;

    // Message pour autoriser les échanges, pas de contenu
    m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd);
    m_send.cmd = BEGIN;

    // Si le client s'est déconnecté
    if(msg_send(fd_circuitV, &m_send, SERVEUR) == 0) connecte = 0;
    
    // #########################################
    // ##########   BOUCLE RECEP MSG   #########
    // #########################################

    while(connecte) {

      printf("\n");

      // Si le client s'est déconnecté
      if(msg_recv(fd_circuitV, &m_recv, SERVEUR) == 0) break;

      // #########################################
      // ############   CMD GETLIST   ############
      // #########################################
      if(m_recv.cmd == GETLIST){
        
        printf("CMD : GETLIST \n");

        // Récupération des informations des fichiers du répertoire de téléchargement
        listdir(PATH_TO_STORAGE_DIR, m_send.content);

        m_send.size = sizeof(m_send.size) + sizeof(m_send.cmd) + strlen(m_send.content);
        m_send.cmd = GETLIST;

        // Si le client s'est déconnecté
        if(msg_send(fd_circuitV, &m_send, SERVEUR) == 0) break;

        printf("GETLIST sent \n");
      }
      // #########################################
      // ##############   CMD GET   ##############
      // #########################################
      else if(m_recv.cmd == GET){

        printf("CMD : GET (%s) \n", m_recv.content);

        // Copie le nom du fichier
        strcpy(nomFichier, m_recv.content);

        // Concaténation du chemin du répertoire de téléchargement et du nom du fichier
        strcpy(cheminFichier, PATH_TO_STORAGE_DIR);
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
        if(msg_send(fd_circuitV, &m_send, SERVEUR) == 0) break;

        // Affiche la taille du fichier, ou l'erreur
        printf("Contenu envoye (size ou erreur) : %s \n", m_send.content);

        // S'il y a eu une erreur on passe à la commande suivante
        if(m_send.cmd == ERROR) continue;

        // Si le client s'est déconnecté
        if(msg_recv(fd_circuitV, &m_recv, SERVEUR) == 0) break;

        // Si la commande n'est pas l'accusé de récéption de la taille du fichier
        if(m_recv.cmd != ACK_SIZE){
          printf("Erreur : cmd %d attendu, cmd %d recu \n", ACK_SIZE, m_recv.cmd);
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

          resSend = msg_send(fd_circuitV, &m_send, SERVEUR);

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
        if(msg_recv(fd_circuitV, &m_recv, SERVEUR) == 0) break;

        // Si la commande n'est pas l'accusé de récéption du fichier
        if(m_recv.cmd != ACK_CONTENT_FILE){
          printf("Erreur : cmd %d attendu, cmd %d recu \n", ACK_CONTENT_FILE, m_recv.cmd);
          continue;
        }

        printf("Le fichier %s a bien ete transmis. \n", nomFichier);

      }
      // #########################################
      // ############   CMD INCONNUE   ###########
      // #########################################
      else{

        printf("Commande inconnue \n");

      }
    }

    printf("Un client vient de se deconnecter. (socket : %d) \n", fd_circuitV);
    nb_clients--;
    connecte = 0;
  }

  // Fermeture de la socket
  if(shutdown(fd_brPublique, SHUT_RDWR) == -1) perror("Erreur shutdown (main) ");
  if(close(fd_brPublique) == -1) perror("Erreur close (main) ");

  printf("\nServeur deconnecte\n");
  printf("\n");

  return EXIT_SUCCESS;
}