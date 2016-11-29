#include "client.h"

void afficherCommandes(){
  printf("\nCommandes disponibles : \n");
  printf("----------------------- \n\n");
  printf("GETLIST \n - Retourne la liste des fichiers telechargeables du serveur \n\n");
  printf("GET <nomFichier> [-DIRL repertoireLocal] \n - Telecharge le fichier nomFichier dans repertoireLocal (./ si vide) \n\n");
  printf("QUIT \n - Quitte le programme \n\n");
}

int main(int argc, char **argv){

  if(argc != 3){
    printf("Usage : client IPServeur|NDDServeur PortServeur \n");
    return EXIT_FAILURE;
  }

  /*
    Récupère l'IP à partir d'un nom de domaine
  */
  char *realIP;
  struct hostent *host;
  if((host = gethostbyname(argv[1])) == NULL){
    perror("Erreur gethostbyname() ");
    return EXIT_FAILURE;
  }
  struct in_addr **ip = (struct in_addr **) host->h_addr_list;
  realIP = inet_ntoa(*ip[0]);

  /*
    Création de la socket TCP
  */
  int localSocket;
  if((localSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Erreur socket() ");
    return EXIT_FAILURE;
  }

  /*
    Connexion à la socket
  */
  struct sockaddr_in clientAddr;
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = inet_addr(realIP);
  clientAddr.sin_port = htons(atoi(argv[2]));
  if(connect(localSocket, (struct sockaddr *) &clientAddr, sizeof clientAddr) == -1){
    perror("Erreur connect() ");
    return EXIT_FAILURE;
  }

  printf("En attente de connexion... \n");

  struct msg msg;
  int sizeRcvTotal = 0;
  int sizeRcv = 0;

  do{
    if((sizeRcv = recv(localSocket, &msg + sizeRcvTotal, sizeof(msg) - sizeRcvTotal, 0)) == -1){
      perror("Erreur recv() ");
      close(localSocket);
      return EXIT_FAILURE;
    }
    sizeRcvTotal += sizeRcv;
    //printf("size rcv total = %d\n", sizeRcvTotal);
  } while(sizeRcvTotal < sizeof(msg.msg_size));

  //printf("msg_size = %d \n", msg.msg_size);
  
  while(sizeRcvTotal < msg.msg_size) {
    if((sizeRcv = recv(localSocket, &msg + sizeRcvTotal, sizeof(msg) - sizeRcvTotal, 0)) == -1){
      perror("Erreur recv() ");
      close(localSocket);
      return EXIT_FAILURE;
    }
    sizeRcvTotal += sizeRcv;
  } 
  
  if(msg.cmd != BEGIN){
    printf("Connexion refusee, veuillez reessayer plus tard \n");
    close(localSocket);
    return EXIT_FAILURE;
  }

  printf("Connexion acceptee \n");

  char cmd[SIZE_BUF_CMD];
  int sizeSendTotal = 0;
  int sizeSend;

  while(1){

    memset(cmd, 0, sizeof(cmd));

    printf("\nEntrez une commande (HELP pour avoir la liste) : \n");

    fgets(cmd, sizeof(cmd), stdin);
    if(cmd[0] == '\n') break;
    cmd[strlen(cmd) - 1] = '\0';

    if(strcmp(cmd, "HELP") == 0){

      afficherCommandes();

    }
    else if(strcmp(cmd, "QUIT") == 0){
      printf("\nAu revoir :)\n");
      printf("------------\n");
      printf("\n");
      break;

    }
    else if(strcmp(cmd, "GETLIST") == 0){
      
      msg.cmd = GETLIST;
      msg.msg_size = sizeof(msg);

      // ENVOI DE LA CMD GETLIST
      while(sizeSendTotal < sizeof(msg)) {
	if((sizeSend = send(localSocket, &msg + sizeSendTotal, sizeof(msg) - sizeSendTotal , 0)) == -1){
	  perror("Erreur send() ");
	}
	sizeSendTotal += sizeSend;
      }

      // RECEPTION DU RESULTAT DE GETLIST
      sizeRcvTotal = 0;
      do{
	if((sizeRcv = recv(localSocket, &msg + sizeRcvTotal, sizeof(msg) - sizeRcvTotal, 0)) == -1){
	  perror("Erreur recv() ");
	  close(localSocket);
	  return EXIT_FAILURE;
	}
	sizeRcvTotal += sizeRcv;
	//printf("size rcv total = %d\n", sizeRcvTotal);
      } while(sizeRcvTotal < sizeof(msg.msg_size));

      //printf("msg_size = %d", msg.msg_size);
      
      while(sizeRcvTotal < msg.msg_size) {
	if((sizeRcv = recv(localSocket, &msg + sizeRcvTotal, sizeof(msg) - sizeRcvTotal, 0)) == -1){
	  perror("Erreur recv() ");
	  close(localSocket);
	  return EXIT_FAILURE;
	}
	sizeRcvTotal += sizeRcv;
      } 

      // AFFICHAGE DU RESULTAT
      printf("\nListe des fichiers du serveur : \n");
      printf("-------------------------------\n\n");
      printf("%s\n", msg.msg_content);
      
    }
    else if(strspn(cmd, "GET") == 3){

      msg.cmd = GET;
      
      char* tableauNomsFichiers = strtok(cmd, " ");

      char DIRLocal[256] = "./";

      while(tableauNomsFichiers != NULL){
	if(strcmp(tableauNomsFichiers, "-DIRL") == 0){
	  tableauNomsFichiers = strtok(NULL, " ");
	  if(tableauNomsFichiers != NULL){
	    strcpy(DIRLocal, tableauNomsFichiers);
	  }
	  else{
	    printf("[-DIRL repertoireLocal] \n");
	    continue;
	  }
	}

	tableauNomsFichiers = strtok(NULL, " ");
      }

      do{
	if((sizeSend = send(localSocket, &msg + sizeSendTotal, strlen(cmd), 0)) == -1){
	  perror("Erreur send() ");
	  continue;
	}
	sizeSendTotal += sizeSend;
      }while(sizeSendTotal < sizeof(msg.cmd));

    }
    else{

      printf("Commande inconnue \n");

    }

  }

  close(localSocket);

  return EXIT_SUCCESS;
}
