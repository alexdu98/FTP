#include "serveur.h"

int main(int argc, char * argv[]) {

  if(argc > 3 || argc < 2 || (atoi(argv[1]) != 0 && atoi(argv[1]) <= 1024)){
    printf("Usage : mainS <port>(0 = rand, > 1024) [repertoireDL] \n");
    return EXIT_FAILURE;
  }

  // #########################################
  // #####   DECLARATION DES VARIABLES   #####
  // #########################################
  
  // Descripteur de la socket publique
  int fd_brPublique;

  // Descripteur circuit virtuel
  int fd_circuitV;
  
  // Configuration de la socket publique et virtuelle
  struct sockaddr_in brPublique;
  struct sockaddr_in brCv;

  // Taille des scrutures sockaddr_in
  socklen_t lgbrCv = sizeof(brCv);
  socklen_t length;

  // Repertoire contenant les fichiers pouvant être téléchargés
  char* path_to_storage_dir;
  
  // #########################################
  // ###   REPERTOIRE DE TELECHARGEMENT   ####
  // #########################################
  
  if(argc == 3){
    if(access(argv[2], F_OK) == 0){
      path_to_storage_dir = argv[2];
      if(argv[2][strlen(argv[2]) - 1] != '/'){
        char nomTemp[256];
        strcpy(nomTemp, argv[2]);
        strcat(nomTemp, "/");
        path_to_storage_dir = nomTemp;
      }
    }
    else{
      printf("'%s' \n", argv[2]);
      perror("Repertoire de telechargement non valide ");
      return EXIT_FAILURE;
    }
  }
  else if(argc == 2){
    path_to_storage_dir = "../repDL/";
  }

  printf("Repertoire de telechargement : %s \n", path_to_storage_dir);

  // #########################################
  // ########   INIT DE LA CONNEXION   #######
  // #########################################

  // Creation de la boite publique
  fd_brPublique = socket(PF_INET, SOCK_STREAM, 0);
  if (fd_brPublique == -1) {
    perror("Erreur socket ");
    exit(EXIT_FAILURE);
  }

  // Configuration de la socket
  brPublique.sin_family = AF_INET;
  brPublique.sin_addr.s_addr = INADDR_ANY;
  brPublique.sin_port = htons(atoi(argv[1])); // num de port choisi par le systeme si 0

  // Liaison de la socket et de la structure
  if(bind(fd_brPublique, (struct sockaddr * ) & brPublique, sizeof(brPublique)) == -1){
    perror("Erreur bind ");
    exit(EXIT_FAILURE);
  }

  // Affiche le numéro de port pour les clients
  length = sizeof(brPublique);
  getsockname(fd_brPublique, (struct sockaddr * ) & brPublique, & length);
  printf("Adresse : %s / Port : %u \n", inet_ntoa(brPublique.sin_addr), ntohs(brPublique.sin_port));

  // Création d'une file d'attente des connexions
  if(listen(fd_brPublique, LG_FILE_ATTENTE) == -1){
    perror("Erreur listen ");
    exit(EXIT_FAILURE);
  }

  /****************************************************************
   ***************    TRAITEMENT DU CLIENT     ********************
   ****************************************************************/

  // Tableau des clients et tableaux des proprietes liees aux clients
  pthread_t clients [NB_MAX_CLIENTS] = {0};
  struct client_args c_args [NB_MAX_CLIENTS];

  int id_client = 0;
  
  // Initialisation des vars partagees
  struct shared_vars s_vars;
  
  s_vars.path_to_storage_dir = path_to_storage_dir; 
  s_vars.nb_clients = 0;
 
  if(pthread_mutex_init(&(s_vars.lock), NULL) != 0)
    perror("pth mutex init shared vars lock ");

  /* File qui sert a determiner quelle place un client
   * qui vient de se connecter peut prendre dans
   * le tableau des clients */
  for(int i = 0; i < NB_MAX_CLIENTS; i++) {
    s_vars.file_places_libres[i] = i;
  }
  
  // Position de fin de file
  s_vars.fin_de_file = NB_MAX_CLIENTS - 1;

  int ret_pth_create = 0, ret_m_lock = 0, ret_m_unlock = 0;
  int ret_shutdown = 0, ret_close = 0;

  // Compte le nombre de fichiers téléchargeable du serveur
  unsigned int nbFiles = countFiles(path_to_storage_dir);

  // Alloue un tableau de structure du nombre de fichier pour compter le nombre de téléchargement
  struct compteur_dl* cpt = malloc(sizeof(struct compteur_dl) * nbFiles);

  // Initialise à 0 le nombre de téléchargement pour chaque fichier
  setCpt(cpt, path_to_storage_dir);

  pthread_t console;
  struct cpt_args cpt_args;
  cpt_args.cpt = cpt;
  cpt_args.nbFiles = &nbFiles;

  if(pthread_mutex_init(&(cpt_args.lock_cpt), NULL) != 0)
    perror("pth mutex init shared vars lock ");

  s_vars.cpt = &cpt_args;

  // Création du thread pour lire la console serveur et afficher le nombre de téléchargement
  pthread_create(&console, NULL, &thread_console_serveur, (void*)&cpt_args);

  // ################################################
  // ##########   BOUCLE LANCEMENT CLIENT   #########
  // ################################################

  printf("\n");

  while(1) {

    // SI LE SERVEUR PEUT ENCORE ACCEPTER DES CLIENTS
    if (s_vars.nb_clients < NB_MAX_CLIENTS) {

      printf("En attente d'un client...\n");

      // Acceptation d'une connexion de client
      fd_circuitV = accept(fd_brPublique, (struct sockaddr * ) & brCv, & lgbrCv);
      if (fd_circuitV == -1) {
	       perror("accept ");
      }
      else { // Traitement normal du client

      	// Operation en ecriture sur la var partagee nb de clients, var a proteger
      	ret_m_lock = pthread_mutex_lock(&(s_vars.lock));
      	if(ret_m_lock != 0)
      	  perror("mutex lock nb clients inc ");

      	// Augmente le nb de clients apres acceptation
      	s_vars.nb_clients++;

      	// Recupere un num de place libre
      	id_client = pop_file(s_vars.file_places_libres, s_vars.fin_de_file);

      	// la file de places libres avance
      	s_vars.fin_de_file--;
      	
      	ret_m_unlock = pthread_mutex_unlock(&(s_vars.lock));
      	if(ret_m_unlock != 0)
      	  perror("mutex unlock nb clients inc ");

            
      	// Initialisation des args qui vont etre passes au thread client
      	c_args[id_client].id_client = id_client;
      	c_args[id_client].fd_circuitV = fd_circuitV;
      	c_args[id_client].s_vars = &s_vars;
      	
      	ret_pth_create = pthread_create(&clients[id_client], NULL, &thread_client, (void*)&c_args[id_client]);
            
      	if(ret_pth_create != 0) {
      	  perror("pth_create ");
      	  // Envoi msg erreur de traitement au client + shutdown close
      	}
      }
    }
    // SI LE SERVEUR A ATTEINT SA LIMITE DE CLIENTS A TRAITER
    else {
      sleep(1);
    }    
  }

  // Ferme la communication entre sockets
  ret_shutdown = shutdown(fd_brPublique, SHUT_RDWR);
  if (ret_shutdown == -1) perror("shutdown ");

  // Ferme le descripteur de la socket
  ret_close = close(fd_brPublique);
  if (ret_close == -1) perror("close ");

  free(cpt);

  return EXIT_SUCCESS;
}
