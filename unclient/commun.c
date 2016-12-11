#include "commun.h"

/* Envoit un message sur une socket
 *
 * @PARAM fd_circuitV, le descripteur de la socket
 * @PARAM m_send, la struct à envoyer
 * @PARAM appelant, SERVEUR ou CLIENT
 * 
 * @RETURN le nombre d'octets envoyes si tout se passe bien,
 * ferme le programme sinon pour le client et retourne l'erreur pour le serveur.
 */
int msg_send(int fd_circuitV, struct msg* m_send, int appelant) {

  int ret_send = 0, ret_shutdown = 0, ret_close = 0;
  int s_total_size = 0;
  char *sendPtr = (char *) m_send;

  // Tant que l'on a pas envoyé tout le message
  while (s_total_size < m_send->size) {

    //printf("SEND :\n %d \n %d \n %s \n", m_send->size, m_send->cmd, m_send->content);
    ret_send = send(fd_circuitV, sendPtr + s_total_size, m_send->size - s_total_size, 0);
    //printf("SEND OK (%d) \n", ret_send);

    if(ret_send < 1){

      // S'il y a une erreur
      if(ret_send == -1) {
        perror("Erreur send (msg_send) ");
      }
      // S'il y a une déconnexion inattendue
      else if(ret_send == 0 && appelant == CLIENT) {
        printf("Connexion fermee par le serveur (msg_send) \n");
      }

      if((ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR)) == -1) perror("Erreur shutdown (msg_send) ");

      if((ret_close = close(fd_circuitV)) == -1) perror("Erreur close (msg_send) ");

      if(appelant == CLIENT)
        exit(EXIT_FAILURE);
      else
        return ret_send;
    }

    s_total_size += ret_send;
  }
  
  return s_total_size;
}


/* Recoit un message sur une socket
 *
 * @PARAM fd_circuitV, le descripteur de la socket
 * @PARAM m_send, la struct à recevoir
 * @PARAM appelant, SERVEUR ou CLIENT
 * 
 * @RETURN le nombre d'octets reçus si tout se passe bien,
 * ferme le programme sinon pour le client et retourne l'erreur pour le serveur.
 */
int msg_recv(int fd_circuitV, struct msg* m_recv, int appelant) {

  int ret_recv = 0, ret_shutdown = 0, ret_close = 0;
  int r_total_size = 0;
  char *recvPtr = (char *) m_recv;

  // Reception de l'entete du msg : la taille totale et la commande
  while (r_total_size < sizeof(m_recv->size) + sizeof(m_recv->cmd)) {
    
    ret_recv = recv(fd_circuitV, recvPtr + r_total_size, sizeof(m_recv->size) + sizeof(m_recv->cmd) - r_total_size, 0);

    if(ret_recv < 1){
    
      // S'il y a une erreur
      if(ret_recv == -1) {
        perror("Erreur recv size + cmd (msg_recv) ");
      }
      // S'il y a une déconnexion inattendue
      else if(ret_recv == 0 && appelant == CLIENT) {
        printf("Connexion fermee par l'hote distant (msg_recv) \n");           
      }

      if((ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR)) == -1) perror("Erreur shutdown (msg_recv) ");

      if((ret_close = close(fd_circuitV)) == -1) perror("Erreur close (msg_recv) ");

      if(appelant == CLIENT)
        exit(EXIT_FAILURE);
      else
        return ret_recv;
    }

    r_total_size += ret_recv;
  }

  //printf("RECV SIZE + CMD :\n %d \n %d \n %d/%d \n", m_recv->size, m_recv->cmd, r_total_size, m_recv->size);

  // Taille et commande du message reçu, maintenant récéption du contenu
  while(r_total_size < m_recv->size) {

    ret_recv = recv(fd_circuitV, recvPtr + r_total_size, m_recv->size - r_total_size, 0);

    if(ret_recv < 1){
      
      // S'il y a une erreur
      if(ret_recv == -1) {
        perror("Erreur recv content (msg_recv) ");
        if((ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR)) == -1) perror("Erreur shutdown (msg_recv) ");
      }
      // S'il y a une déconnexion inattendue
      else if(ret_recv == 0 && appelant == CLIENT) {
        printf("Connexion fermee par l'hote distant (msg_recv) \n");           
      }

      if((ret_close = close(fd_circuitV)) == -1) perror("Erreur close (msg_recv) ");

      if(appelant == CLIENT)
        exit(EXIT_FAILURE);
      else
        return ret_recv;
    }

    r_total_size += ret_recv;
  }

  // Si le content n'est pas completement rempli, on place le \0 à la fin
  if(r_total_size < sizeof(*m_recv))
    m_recv->content[r_total_size - (sizeof(m_recv->size) + sizeof(m_recv->cmd))] = '\0';
  
  //printf("RECV CONTENT :\n %s \n %d/%d \n", m_recv->content, r_total_size, m_recv->size);
  return r_total_size;
}
