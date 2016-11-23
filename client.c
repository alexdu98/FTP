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
	printf("GETLIST : retourne la liste des fichiers telechargeables du serveur \n");
	printf("GET <nomFichier> [repertoireLocal] : telecharge le fichier nomFichier dans repertoireLocal (./ si vide) \n");
	printf("QUIT : quitte le programme \n");
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
		if((sizeRcv = recv(localSocket, msg + sizeRcvTotal, sizeof(msg), 0)) == -1){
			perror("Erreur recv() ");
			close(localSocket);
			return EXIT_FAILURE;
		}
		sizeRcvTotal += sizeRcv;
	}while(sizeRcvTotal < (sizeof(int) * 2));

	if(strcmp(responseAccept, "BEGIN") != 0){
		printf("Connexion refusee, veuillez reessayer plus tard \n");
		close(localSocket);
		return EXIT_FAILURE;
	}

	printf("Connexion acceptee \n");

	char cmd[SIZE_BUF_CMD];

	while(1){

		memset(cmd, 0, sizeof(cmd));

		printf("\nEntrez une commande (help pour avoir la liste) : \n");

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
			if(send(localSocket, cmd, strlen(cmd), 0) == -1){
				perror("Erreur send() ");
				continue;
			}

		}
		else if(strspn(cmd, "GET") == 3){

		}
		else{
			printf("Commande inconnue \n");
		}

	}

	close(localSocket);

	return EXIT_SUCCESS;
}