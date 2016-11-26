// gcc -o client client.c
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <libgen.h>

#define BEGIN 0
#define GETLIST 1
#define GET 2

#define SIZE_BUF_CMD 2048

struct msg{
	int cmd;
	int msg_size;
	char content[256];
};

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
	int sizeRcv;
	do{
		if((sizeRcv = recv(localSocket, &msg, sizeof(msg), 0)) == -1){
			perror("Erreur recv() ");
			close(localSocket);
			return EXIT_FAILURE;
		}
		sizeRcvTotal += sizeRcv;
	}while(sizeRcvTotal < sizeof(msg.cmd));

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

			break;

		}
		else if(strcmp(cmd, "GETLIST") == 0){

			msg.cmd = GETLIST;

			do{
				if((sizeSend = send(localSocket, cmd, strlen(cmd), 0)) == -1){
					perror("Erreur send() ");
					continue;
				}
				sizeSendTotal += sizeSend;
			}while(sizeSendTotal < sizeof(msg.cmd));

			do{
				if((sizeRcv = recv(localSocket, &msg, sizeof(msg), 0)) == -1){
					perror("Erreur recv() ");
					close(localSocket);
					return EXIT_FAILURE;
				}
				sizeRcvTotal += sizeRcv;
			}while(sizeRcvTotal < sizeof(msg.cmd));

		}
		else if(strspn(cmd, "GET") == 3){

			msg.cmd = GET;

			tableauNomsFichiers = strtok(cmd, " ");

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
				if((sizeSend = send(localSocket, cmd, strlen(cmd), 0)) == -1){
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