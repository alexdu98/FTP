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


	// #########################################
	// ########   INIT DE LA CONNEXION   #######
	// #########################################

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

	// Si la première commande n'est pas BEGIN, connexion refusée
	if(msgRecv.cmd != BEGIN){
		printf("Connexion refusee, veuillez reessayer plus tard \n");
		close(localSocket);
		return EXIT_FAILURE;
	}

	printf("Connexion acceptee \n");


	// #########################################
	// ##########   BOUCLE PRINCIPALE   ########
	// #########################################

	while(1){

		memset(cmd, 0, sizeof(cmd));

		printf("\nEntrez une commande (HELP pour avoir la liste) : \n");

		// Demande au client de rentrer une commande
		fgets(cmd, sizeof(cmd), stdin);
		if(cmd[0] == '\n') break;
		cmd[strlen(cmd) - 1] = '\0';

		// Affiche les commandes disponibles
		if(strcmp(cmd, "HELP") == 0){

			afficherCommandes();

		}
		// Ferme la socket et quitte le programme
		else if(strcmp(cmd, "QUIT") == 0){
			shutdown(localSocket, SHUT_RDWR);
			close(localSocket);
			printf("\nAu revoir :)\n");
			printf("------------\n");
			printf("\n");
			break;

		}
		// Affiche la liste des fichiers disponibles sur le serveur
		else if(strcmp(cmd, "GETLIST") == 0){
			
			msgSend.cmd = GETLIST;
			// Pas de contenu
			msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);

			int ret_m_send = msg_send(localSocket, &msgSend, 0);

			memset(msgRecv.content, 0, sizeof(msgRecv.content));
			int res_recv = msg_recv(localSocket, &msgRecv, 0);

			printf("\nListe des fichiers du serveur : \n");
			printf("-------------------------------\n\n");
			printf("%s\n", msgRecv.content);

		}
		// Télécharge un ou plusieurs fichiers
		else if(strncmp(cmd, "GET", 3) == 0){

			msgSend.cmd = GET;

			// Répertoire de destination
			char DIRLocal[256] = "";

			// Commande client qui sera modifié par strtok
			char copyCmd[256];
			strcpy(copyCmd, cmd);

			// Liste des fichiers
			char copyFiles[256];
			strcpy(copyFiles, cmd + 4);

			// Découpe la commande sur les espaces
			char* tabCmdGet = strtok(copyCmd, " ");

			// La taille de la commande entre le GET et le potentiel -DIRL
			int taille = -6;

			// Tant qu'on a pas inspecter toute la commande découpé
			while(tabCmdGet != NULL){

				// On ajoute la taille de la partie de la commande découpé
				taille += strlen(tabCmdGet);
				
				// Si la partie découpé contient le paramètre -DIRL
				if(strcmp(tabCmdGet, "-DIRL") == 0){

					// Place le caractère de fin de chaine pour séparer la liste des fichiers
					copyFiles[taille] = '\0';

					// Passe à la chaine suivante (répertoire de destination)
					tabCmdGet = strtok(NULL, " ");

					if(tabCmdGet != NULL){

						// Copie le répertoire de destination dans la chaine
						strcpy(DIRLocal, tabCmdGet);

						// Si le / de fin n'est pas présent on l'ajoute
						if(DIRLocal[strlen(DIRLocal)] != '/'){
					        strcat(DIRLocal, "/");
					    }
						break;
					}
					// Chaine vide -> erreur
					else{
						printf("[-DIRL repertoireLocal] \n");
						break;
					}
				}

				// Passe à la chaine suivante
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
		// Commande inconnue
		else{

			printf("Commande inconnue \n");

		}

	}

	close(localSocket);

	return EXIT_SUCCESS;
}