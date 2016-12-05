#include "serveur.c"

// Affiche les commandes disponibles
void afficherCommandes(){
	printf("\nCommandes disponibles : \n");
	printf("----------------------- \n\n");
	printf("GETLIST \n - Retourne la liste des fichiers telechargeables du serveur \n\n");
	printf("GET <fichier1> [<fichier2> ...] [-DIRL repertoireLocal] \n - Telecharge le(s) fichier(s) fichierX dans repertoireLocal (./ si vide) \n\n");
	printf("QUIT \n - Quitte le programme \n\n");
}