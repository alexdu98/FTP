#include "commun.h"

/*
 * @PARAM fd_circuitV, le descripteur de la socket associer
 * a l'envoi de msg
 * @PARAM m_send, la struct a envoyer par message
 * 
 * @RETURN le nombre d'octets envoyes si tout se passe bien,
 * 0 si le pair se deconnecte, et -1 en cas d'erreur.
 */
int msg_send(int fd_circuitV, struct msg* m_send, int onlyContent) {

  int ret_send = 0, ret_shutdown = 0, ret_close = 0;
  int s_total_size = 0;

  void* sendS;
  sendS = (void*)m_send;

  unsigned int tailleMsg;
  tailleMsg = m_send->size;

  if(onlyContent > 0){
    sendS = (void*)m_send->content;
  }

  //printf("*cmd : %d \n", ((struct msg*)sendS)->cmd);
  //printf("*size : %d \n", ((struct msg*)sendS)->size);
  //printf("*content1 : %s \n", ((struct msg*)sendS)->content);
  //printf("*content2 : %s \n", (char*)sendS);
  //printf("*s_total_size : %d \n", s_total_size);

  // printf("only %d\n", onlyContent);
  // printf("::%p \n", m_send);
  // printf("::%p \n", m_send->content);
  // printf("::%p \n", sendS);

  while (s_total_size < tailleMsg) {

    ret_send = send(fd_circuitV, sendS + s_total_size, tailleMsg - s_total_size, 0);

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


int msg_recv(int fd_circuitV, struct msg* m_recv, int onlyContent) {

  int ret_recv = 0, ret_shutdown = 0, ret_close = 0;
  int r_total_size = 0;

  void* recvS;
  recvS = (void*)m_recv;

  if(onlyContent > 0){
    recvS = (void*)m_recv->content;
  }

  memset(m_recv->content, 0, sizeof(m_recv->content));

  if(onlyContent == 0){

    // Reception des x premiers octets contenant la taille du msg et la commande
    while (r_total_size < sizeof(m_recv->size) + sizeof(m_recv->cmd)) {

      //printf("*%lu \n", sizeof(*m_recv));
      ret_recv = recv(fd_circuitV, recvS + r_total_size, sizeof(m_recv->size) + sizeof(m_recv->cmd) - r_total_size, 0);

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

  }

  unsigned int tailleMsg = m_recv->size;

  // Taille du msg recu, maintenant reception du reste
  while(r_total_size < tailleMsg) {

    ret_recv = recv(fd_circuitV, recvS + r_total_size, tailleMsg - r_total_size, 0);

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