#include "client.h"

int main(int argc, char **argv){

	if(argc != 3){
		printf("Usage : mainC IPServeur|NDDServeur PortServeur \n");
		return EXIT_FAILURE;
	}

	// #########################################
	// #####   DECLARATION DES VARIABLES   #####
	// #########################################

	struct hostent *host;
	struct in_addr **ip;
	struct sockaddr_in clientAddr;
	struct msg msgSend;
	struct msg msgRecv;

	char *realIP;
	char cmd[256];
	
	int localSocket;
	int sizeRcvTotal = 0;
	int sizeRcv = 0;
	int sizeSendTotal = 0;
	int sizeSend;
	

	// Récupère l'IP à partir d'un nom de domaine
	if((host = gethostbyname(argv[1])) == NULL){
		perror("Erreur gethostbyname() ");
		return EXIT_FAILURE;
	}
	ip = (struct in_addr **) host->h_addr_list;
	realIP = inet_ntoa(*ip[0]);

	// Création de la socket TCP
	if((localSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Erreur socket() ");
		return EXIT_FAILURE;
	}
	
	// Configuration de la socket
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr(realIP);
	clientAddr.sin_port = htons(atoi(argv[2]));

	// Connexion à la socket distante
	if(connect(localSocket, (struct sockaddr *) &clientAddr, sizeof clientAddr) == -1){
		perror("Erreur connect() ");
		return EXIT_FAILURE;
	}

	printf("En attente de connexion... \n");

	int res_recv = msg_recv(localSocket, &msgRecv, 0);
	
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
			close(localSocket);
			printf("\nAu revoir :)\n");
			printf("------------\n");
			printf("\n");
			break;

		}
		else if(strcmp(cmd, "GETLIST") == 0){
			
			msgSend.cmd = GETLIST;
			msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);

			// ENVOI DE LA CMD GETLIST
			int ret_m_send = msg_send(localSocket, &msgSend, 0);

			// RECEPTION DU RESULTAT DE GETLIST
			memset(msgRecv.content, 0, sizeof(msgRecv.content));
			int res_recv = msg_recv(localSocket, &msgRecv, 0);

			// AFFICHAGE DU RESULTAT
			printf("\nListe des fichiers du serveur : \n");
			printf("-------------------------------\n\n");
			printf("%s\n", msgRecv.content);

		}
		else if(strncmp(cmd, "GET", 3) == 0){

			msgSend.cmd = GET;
			char DIRLocal[256] = "";

			char copyCmd[256];
			strcpy(copyCmd, cmd);

			char copyFiles[256];
			strcpy(copyFiles, cmd + 4);

			char* tabCmdGet = strtok(copyCmd, " ");

			int taille = -6;
			while(tabCmdGet != NULL){
				taille += strlen(tabCmdGet);
				if(strcmp(tabCmdGet, "-DIRL") == 0){
					copyFiles[taille] = '\0';
					tabCmdGet = strtok(NULL, " ");
					if(tabCmdGet != NULL){
						strcpy(DIRLocal, tabCmdGet);
						if(DIRLocal[strlen(DIRLocal)] != '/'){
			        strcat(DIRLocal, "/");
			      }
						break;
					}
					else{
						printf("[-DIRL repertoireLocal] \n");
						break;
					}
				}
				tabCmdGet = strtok(NULL, " ");
			}

			if(strcmp(DIRLocal,"") == 0)
				strcpy(DIRLocal, "./");

			char* file = strtok(copyFiles, " ");
			while(file != NULL){
				msgSend.cmd = GET;
				strcpy(msgSend.content, file);
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd) + strlen(msgSend.content);

				// ENVOI DE LA CMD GET
				int ret_m_send = msg_send(localSocket, &msgSend, 0);

				// RECEPTION DE LA TAILLE DU FICHIER OU DE L'ERREUR
				int res_recv = msg_recv(localSocket, &msgRecv, 0);
				if(msgRecv.cmd == ERROR){
					printf("Erreur : %s \n", msgRecv.content);
					continue;
				}
				
				unsigned int tailleFichier = atoi(msgRecv.content);

				// ENVOI DE LA CMD ACK_SIZE
				msgSend.cmd = ACK_SIZE;
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);
				ret_m_send = msg_send(localSocket, &msgSend, 0);

				printf("\nTaille du fichier %s : %d \n", file, tailleFichier);

				char nomFichier[255];

        /* Réinitialisation de la variable */
        memset(nomFichier, 0, sizeof(nomFichier));

        strcpy(nomFichier, DIRLocal);
        strcat(nomFichier, file);

      	/* Fichier à remplir */
				FILE* fichier = NULL;
		
				/* On créer/ouvre le fichier */
				if((fichier = fopen(nomFichier, "wb+")) == NULL)
					perror("Erreur fopen() ");

				unsigned int nbCarRecuTotal = 0;
				unsigned int carFseek = 0;
				unsigned int test = 0;
				int nbCarALire = 0;
				int onlyContent = 0;
				while(test < tailleFichier){

					msgRecv.size = (tailleFichier + sizeof(msgRecv.size) + sizeof(msgRecv.cmd)) - nbCarRecuTotal;
					if(msgRecv.size > sizeof(msgRecv.content))
						msgRecv.size = sizeof(msgRecv.content);

					int res_recv = msg_recv(localSocket, &msgRecv, onlyContent);
					//printf("> %d \n", res_recv);
					
					fseek(fichier, carFseek, SEEK_SET);
					unsigned int resfw = 0;

					nbCarALire = sizeof(msgRecv.content);
					
					if(msgRecv.size < sizeof(msgRecv.content)){
						nbCarALire = msgRecv.size;
					}
					if(nbCarALire > tailleFichier)
						nbCarALire = tailleFichier;

					// printf("#%c\n", msgRecv.content[nbCarALire - 9]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 8]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 7]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 6]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 5]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 4]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 3]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 2]);
					// printf("#%c\n", msgRecv.content[nbCarALire - 1]);
					// printf("%d \n", nbCarALire);
					resfw = fwrite(msgRecv.content, 1, nbCarALire, fichier);
					//printf(">> %d \n", resfw);

					test += resfw;
					
					carFseek += resfw;
					nbCarRecuTotal += res_recv;
				
					onlyContent++;
				}

				fclose(fichier);

				printf("Le fichier %s a bien ete telecharge \n", file);

				file = strtok(NULL, " ");
			}

		}
		else{

			printf("Commande inconnue \n");

		}

	}

	close(localSocket);

	return EXIT_SUCCESS;
}