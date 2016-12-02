#include "client.h"

int main(int argc, char **argv){

	if(argc != 3){
		printf("Usage : mainC IPServeur|NDDServeur PortServeur \n");
		return EXIT_FAILURE;
	}

	// ###############################
	// ## DECLARATION DES VARIABLES ##
	// ###############################

	char *realIP;
	struct hostent *host;
	int localSocket;
	struct sockaddr_in clientAddr;
	struct msg msgSend;
	struct msg msgRecv;
	int sizeRcvTotal = 0;
	int sizeRcv = 0;
	int sizeSendTotal = 0;
	int sizeSend;
	char cmd[256];


	/*
		Récupère l'IP à partir d'un nom de domaine
	*/
	if((host = gethostbyname(argv[1])) == NULL){
		perror("Erreur gethostbyname() ");
		return EXIT_FAILURE;
	}
	struct in_addr **ip = (struct in_addr **) host->h_addr_list;
	realIP = inet_ntoa(*ip[0]);

	/*
		Création de la socket TCP
	*/
	
	if((localSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Erreur socket() ");
		return EXIT_FAILURE;
	}

	/*
		Connexion à la socket
	*/
	
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr(realIP);
	clientAddr.sin_port = htons(atoi(argv[2]));
	if(connect(localSocket, (struct sockaddr *) &clientAddr, sizeof clientAddr) == -1){
		perror("Erreur connect() ");
		return EXIT_FAILURE;
	}

	printf("En attente de connexion... \n");

	do{
		if((sizeRcv = recv(localSocket, &msgRecv + sizeRcvTotal, sizeof(msgRecv) - sizeRcvTotal, 0)) == -1){
			perror("Erreur recv() ");
			close(localSocket);
			return EXIT_FAILURE;
		}
		sizeRcvTotal += sizeRcv;
	} while(sizeRcvTotal < sizeof(msgRecv.size));
	
	while(sizeRcvTotal < msgRecv.size) {
		if((sizeRcv = recv(localSocket, &msgRecv + sizeRcvTotal, sizeof(msgRecv) - sizeRcvTotal, 0)) == -1){
			perror("Erreur recv() ");
			close(localSocket);
			return EXIT_FAILURE;
		}
		sizeRcvTotal += sizeRcv;
	} 
	
	if(msgRecv.cmd != BEGIN){
		printf("Connexion refusee, veuillez reessayer plus tard \n");
		close(localSocket);
		return EXIT_FAILURE;
	}

	printf("Connexion acceptee \n");

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
			shutdown(localSocket, SHUT_RDWR);
			printf("\nAu revoir :)\n");
			printf("------------\n");
			printf("\n");
			break;

		}
		else if(strcmp(cmd, "GETLIST") == 0){
			
			msgSend.cmd = GETLIST;
			msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);

			// ENVOI DE LA CMD GETLIST
			int ret_m_send = msg_send(localSocket, &msgSend);

			// RECEPTION DU RESULTAT DE GETLIST
			memset(msgRecv.content, 0, sizeof(msgRecv.content));
			int res_recv = msg_recv(localSocket, &msgRecv);

			// AFFICHAGE DU RESULTAT
			printf("\nListe des fichiers du serveur : \n");
			printf("-------------------------------\n\n");
			printf("%s\n", msgRecv.content);

		}
		else if(strspn(cmd, "GET") == 3){

			msgSend.cmd = GET;

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
				if((sizeSend = send(localSocket, &msgSend + sizeSendTotal, msgSend.size - sizeSendTotal, 0)) == -1){
					perror("Erreur send() ");
					continue;
				}
				sizeSendTotal += sizeSend;
			}while(sizeSendTotal < sizeof(msgSend.cmd));

		}
		else{

			printf("Commande inconnue \n");

		}

	}

	close(localSocket);

	return EXIT_SUCCESS;
}