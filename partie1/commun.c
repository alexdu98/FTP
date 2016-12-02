#include "commun.h"

/*
 * @PARAM fd_circuitV, le descripteur de la socket associer
 * a l'envoi de msg
 * @PARAM m_send, la struct a envoyer par message
 * 
 * @RETURN le nombre d'octets envoyes si tout se passe bien,
 * 0 si le pair se deconnecte, et -1 en cas d'erreur.
 */
int msg_send(int fd_circuitV, struct msg* m_send) {

  int ret_send = 0, ret_shutdown = 0, ret_close = 0;
  int s_total_size = 0;

  while (s_total_size < sizeof(m_send->size)) {		

    ret_send = send(fd_circuitV, m_send + s_total_size, m_send->size - s_total_size, 0);

    if(ret_send == -1) {
      perror("send msg_send ");
      return 0;
    }
    // Si le client se deconnecte intempestivement
    else if(ret_send == 0) {   
      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      if(ret_shutdown == -1) perror("msg_send shutdown ");

      // Ferme le descripteur de la socket
      ret_close = close(fd_circuitV);
      if(ret_close == -1) perror("msg_send close ");
      return 0;        			       
    }

    s_total_size += ret_send;
  }
  return s_total_size;

}


int msg_recv(int fd_circuitV, struct msg* m_recv) {

  int ret_recv = 0, ret_shutdown = 0, ret_close = 0;
  int r_total_size = 0;

  memset(m_recv->content, 0, sizeof(m_recv->content));

  // Reception des 4 premiers octets contenant la taille du msg
  while (r_total_size < sizeof(m_recv->size)) {

    ret_recv = recv(fd_circuitV, m_recv + r_total_size, sizeof(*m_recv) - r_total_size, 0);

    if(ret_recv == -1) {
      perror("recv cmd ");
      exit(EXIT_FAILURE);
    }
    // Si le client se deconnecte intempestivement
    else if(ret_recv == 0) {   
      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      if(ret_shutdown == -1) perror("shutdown disconnect ");

      // Ferme le descripteur de la socket
      ret_close = close(fd_circuitV);
      if(ret_close == -1) perror("close ");
        return 0;
    }
      	  
    r_total_size += ret_recv;
  }

  // Taille du msg recu, maintenant reception du reste
  while(r_total_size < m_recv->size) {
    printf("%d/%d \n", r_total_size, m_recv->size);
    ret_recv = recv(fd_circuitV, m_recv + r_total_size, sizeof(*m_recv) - r_total_size, 0);

    if(ret_recv == -1) {
      perror("recv cmd ");
      exit(EXIT_FAILURE);
    }
    // Si le client se deconnecte intempestivement
    else if(ret_recv == 0) {   
      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      if(ret_shutdown == -1) perror("shutdown disconnect ");

      // Ferme le descripteur de la socket
      ret_close = close(fd_circuitV);
      if(ret_close == -1) perror("close ");

      return 0;
    }

    r_total_size += ret_recv;
  }

  return r_total_size;

}