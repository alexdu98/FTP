#include "client.h"

int main(int argc, char **argv){

	if(argc != 3){
		printf("Usage : mainC <IPServeur|NDDServeur> <PortServeur> \n");
		return EXIT_FAILURE;
	}

	// #########################################
	// #####   DECLARATION DES VARIABLES   #####
	// #########################################

	// Informations sur le serveur
	struct hostent *host;

	// IP du serveur
	struct in_addr **ip;
	char *realIP;

	// Configuration de la socket
	struct sockaddr_in clientAddr;

	// Message reçu et envoyé
	struct msg msgSend;
	struct msg msgRecv;

	// Taille reçu
	int resRecv;

	// Retour du send
  int resSend;

	// Commande rentré par le client
	char cmd[256];
	
	// Descripteur de la socket
	int localSocket;

	// Retour du shutdown et close
	int retShutdown;
	int retClose;

	// Bouléen de connexion
	int connecte = 0;

	// Taille du fichier à recevoir
	unsigned int tailleFichier;

	// Chemin et nom du fichier à créer
	char pathToOpen[256];

	// Nouveau nom pour le fichier téléchargé
	char newName[256];

	// Fichier rendu par fopen
	FILE* fichier;

	// Offset pour le fseek
	unsigned int offsetFseek;

	// Nombre de caractères à écrire dans le fichier
	int nbCarALire;

	// Résultat du fread, nombre de caractères lu
  unsigned int resfr;

	// Résultat du fwrite, nombre de caractères écrit
	unsigned int resFwrite;


	// #########################################
	// ########   INIT DE LA CONNEXION   #######
	// #########################################

	// Récupère l'IP à partir d'un nom de domaine
	if((host = gethostbyname(argv[1])) == NULL){
		perror("Erreur gethostbyname ");
		return EXIT_FAILURE;
	}
	ip = (struct in_addr **) host->h_addr_list;
	realIP = inet_ntoa(*ip[0]);

	// Création de la socket TCP
	if((localSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Erreur socket ");
		return EXIT_FAILURE;
	}
	
	// Configuration de la socket
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr(realIP);
	clientAddr.sin_port = htons(atoi(argv[2]));

	// Connexion à la socket distante
	if(connect(localSocket, (struct sockaddr *) &clientAddr, sizeof clientAddr) == -1){
		perror("Erreur connect ");
		if((retClose = close(localSocket)) == -1) perror("Erreur close (connect) ");
		return EXIT_FAILURE;
	}

	printf("En attente de connexion... \n");

	resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

	// Si la première commande n'est pas BEGIN, connexion refusée
	if(msgRecv.cmd != BEGIN){
		printf("Connexion refusee, veuillez reessayer plus tard \n");
	}
	else{
		connecte = 1;
		printf("Connexion acceptee");
	}

	// #########################################
	// ##########   BOUCLE PRINCIPALE   ########
	// #########################################

	while(connecte){

		memset(cmd, 0, sizeof(cmd));

		printf("\n\nEntrez une commande (HELP pour avoir la liste) : \n");

		// Demande au client de rentrer une commande
		fgets(cmd, sizeof(cmd), stdin);

		// Si la commande est vide on ferme le programme
		if(cmd[0] == '\n'){
			strcpy(cmd, "QUIT "); // Ne pas enlevé l'espace !
		}

		// On remplace le \n ou l'espace par \0
		cmd[strlen(cmd) - 1] = '\0';

		// #########################################
		// ##############   CMD HELP   #############
		// #########################################
		if(strcmp(cmd, "HELP") == 0){

			afficherCommandes();

		}
		// #########################################
		// ##############   CMD QUIT   #############
		// #########################################
		else if(strcmp(cmd, "QUIT") == 0){

			break;

		}
		// #########################################
		// ############   CMD GETLIST   ############
		// #########################################
		else if(strcmp(cmd, "GETLIST") == 0){
			
			// Pas de contenu
			msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);
			msgSend.cmd = GETLIST;

			msg_send(localSocket, &msgSend, CLIENT);

			printf("\nListe des fichiers du serveur : \n");
			printf("-------------------------------\n\n");

			while(1){

				resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

				if(msgRecv.cmd == ACK_GETLIST){
					break;
				}
				// Si la première commande n'est pas BEGIN, connexion refusée
				else if(msgRecv.cmd != GETLIST){
					printf("Erreur : commande attendu = %d / reçu = %d \n", GETLIST, msgRecv.cmd);
					continue;
				}

				printf("%s", msgRecv.content);
			}

		}
		// #########################################
		// ##############   CMD GET   ##############
		// #########################################
		else if(strncmp(cmd, "GET ", 4) == 0){

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
			int taille = -7;

			// Tant qu'on a pas inspecter toute la commande découpée
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
						printf("[-DIRL <repertoireLocal>] \n");
						break;
					}
				}

				// Passe à la chaine suivante
				tabCmdGet = strtok(NULL, " ");
			}

			// Si pas de répertoire indiqué, on prend le courant
			if(strcmp(DIRLocal,"") == 0)
				strcpy(DIRLocal, "./");

			// Affichage du répertoire de téléchargement local
			printf("Repertoire de telechargement : %s \n", DIRLocal);

			// Découpe la liste de fichiers sur les espaces
			char* file = strtok(copyFiles, " ");

			// Tant qu'on a pas inspecter toute la liste de fichiers
			while(file != NULL){
				
				// On met le nom du fichier dans le contenu
				strcpy(msgSend.content, file);
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd) + strlen(msgSend.content);
				msgSend.cmd = GET;

				msg_send(localSocket, &msgSend, CLIENT);

				resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

				// Si on reçoit une erreur
				if(msgRecv.cmd == ERROR){
					printf("\nErreur : %s \n", msgRecv.content);
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}
				// Si on reçoit une commande différente de GET
				else if(msgRecv.cmd != SIZE){
					printf("\nErreur : commande attendu = %d / reçu = %d \n", SIZE, msgRecv.cmd);
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}
				
				// Transformation de la chaine en entier
				tailleFichier = atoi(msgRecv.content);

				printf("\nTaille du fichier %s : %d \n", file, tailleFichier);

				// Accusé de récéption, pas de contenu
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);
				msgSend.cmd = ACK_SIZE;
				msg_send(localSocket, &msgSend, CLIENT);

				// On copie le chemin du répertoire de destination
        strcpy(pathToOpen, DIRLocal);

        // On concatene le nom du fichier
        strcat(pathToOpen, file);

        // On test l'existence du fichier, si existe on propose de renommer
        while(access(pathToOpen, F_OK) != -1){

        	printf("Un fichier portant ce nom existe deja dans le repertoire de destination\n");
        	printf("Entrez un nouveau nom ou appuyez sur entree pour ecraser le contenu\n");
        	printf("Nom du nouveau fichier [] : ");

        	// Demande au client de rentrer une commande
					fgets(newName, sizeof(newName), stdin);

					printf("\n");

					// Vide => ecrase le contenu
					if(newName[0] == '\n'){
						break;
					}

					// Supprime le saut de ligne
					newName[strlen(newName) - 1] = '\0';

					// Modifie le nom du nouveau fichier
					strcpy(pathToOpen, DIRLocal);
					strcat(pathToOpen, newName);
        }

				fichier = NULL;
		
				// Création/Ouverture du fichier
				if((fichier = fopen(pathToOpen, "wb+")) == NULL){
					perror("Erreur fopen ");
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}

				offsetFseek = 0;
				while(offsetFseek < tailleFichier){

					resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

					// Si on reçoit une commande différente de CONTENT_FILE
					if(msgRecv.cmd != CONTENT_FILE){
						printf("Erreur : commande attendu = %d / reçu = %d \n", CONTENT_FILE, msgRecv.cmd);
						break;
					}
					
					// Place le curseur pour la prochaine écriture
					if(fseek(fichier, offsetFseek, SEEK_SET) == -1){
						perror("Erreur fseek ");
						break;
					}

					// Nombre de caractères à écrire dans le fichier
					nbCarALire = resRecv - sizeof(msgRecv.size) - sizeof(msgRecv.cmd);

					// Ecriture dans le fichier
					resFwrite = fwrite(msgRecv.content, 1, nbCarALire, fichier);
					
					// Decale l'offset
					offsetFseek += resFwrite;
				}

				if(fclose(fichier) == EOF){
					perror("Erreur fclose ");
					break;
				}

				// Envoi de l'accusé de récéption du fichier
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd);
				msgSend.cmd = ACK_CONTENT_FILE;

				msg_send(localSocket, &msgSend, CLIENT);

				printf("Le fichier %s a bien ete telecharge \n", file);

				// Passe au fichier suivant
				file = strtok(NULL, " ");
			}

		}
		// #########################################
		// #############   CMD SEND   ##############
		// #########################################
		else if(strncmp(cmd, "SEND ", 5) == 0){

			// Liste des fichiers
			char copyFiles[256];
			strcpy(copyFiles, cmd + 5);

			// Découpe la liste de fichiers sur les espaces
			char* file = strtok(copyFiles, " ");

			// Tant qu'on a pas inspecter toute la liste de fichiers
			while(file != NULL){

				// Si on ne peut pas accéder au fichier on affiche l'erreur
	      if((fichier = fopen(file, "rb")) == NULL){
	      	printf("\nErreur SEND %s ", file);
					perror("");
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
	      }
	      // S'il n'y a pas d'erreur on calcul la taille du fichier
	      else{
	      	fseek(fichier, 0, SEEK_END);
	      	tailleFichier = ftell(fichier);
	      }

	      char* nomFichier = basename(file);

				// On met le nom du fichier dans le contenu
				strcpy(msgSend.content, nomFichier);
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd) + strlen(msgSend.content);
				msgSend.cmd = SEND;

				msg_send(localSocket, &msgSend, CLIENT);

				resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

				// Si on reçoit une erreur
				if(msgRecv.cmd == ERROR){
					printf("\nErreur : %s \n", msgRecv.content);
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}
				// Si on reçoit une commande différente de GET
				else if(msgRecv.cmd != SEND){
					printf("\nErreur : commande attendu = %d / reçu = %d \n", SEND, msgRecv.cmd);
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}

				printf("\nTaille du fichier %s : %d \n", nomFichier, tailleFichier);

				sprintf(msgSend.content, "%u", tailleFichier);
				msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd) + strlen(msgSend.content);
				msgSend.cmd = SIZE;

				msg_send(localSocket, &msgSend, CLIENT);

				resRecv = msg_recv(localSocket, &msgRecv, CLIENT);

				// Si on reçoit une commande différente de ACK_SIZE
				if(msgRecv.cmd != ACK_SIZE){
					printf("\nErreur : commande attendu = %d / reçu = %d \n", ACK_SIZE, msgRecv.cmd);
					// Passe au fichier suivant
					file = strtok(NULL, " ");
					continue;
				}

				msgSend.cmd = CONTENT_FILE;
				offsetFseek = 0;
	      while(offsetFseek < tailleFichier){

	      	// Place le curseur pour la prochaine lecture
	      	if(fseek(fichier, offsetFseek, SEEK_SET) == -1){
	      	  perror("Erreur fseek ");
	      	  break;
	      	}

	      	// Lecture du fichier
	      	resfr = fread(msgSend.content, 1, sizeof(msgSend.content), fichier);

	      	// On envoit la taille que l'on a lu
	      	msgSend.size = sizeof(msgSend.size) + sizeof(msgSend.cmd) + resfr;

	      	resSend = msg_send(localSocket, &msgSend, SERVEUR);

	      	// Decale l'offset
	      	offsetFseek += resSend - (sizeof(msgSend.size) + sizeof(msgSend.cmd));
	      }

				if(fclose(fichier) == EOF){
					perror("Erreur fclose ");
					break;
				}

     		msg_recv(localSocket, &msgRecv, SERVEUR);

				// Si la commande n'est pas l'accusé de récéption du fichier
	      if(msgRecv.cmd != ACK_CONTENT_FILE){
	      	printf("Erreur : cmd %d attendu, cmd %d recu (socket : %d) \n", ACK_CONTENT_FILE, msgRecv.cmd, localSocket);
	      	continue;
      	}

				printf("Le fichier %s a bien ete televerse \n", nomFichier);

				// Passe au fichier suivant
				file = strtok(NULL, " ");
			}

		}
		// #########################################
		// ############   CMD INCONNUE   ###########
		// #########################################
		else{

			printf("Commande inconnue \n");

		}

	}

	// Fermeture de la socket
	if((retShutdown = shutdown(localSocket, SHUT_RDWR)) == -1) perror("Erreur shutdown (main) ");
	if((retClose = close(localSocket)) == -1) perror("Erreur close (main) ");

	printf("\nAu revoir :)\n");
	printf("------------\n");
	printf("\n");

	return EXIT_SUCCESS;
}