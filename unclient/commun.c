#include "commun.h"

/* Envoit un message sur une socket
 *
 * @PARAM fd_circuitV, le descripteur de la socket
 * @PARAM m_send, la struct à envoyer
 * @PARAM onlyContent, 0 envoi du message complet (size, cmd, content)
 * > 0 envoi du content seulement
 * 
 * @RETURN le nombre d'octets envoyes si tout se passe bien,
 * 0 si le pair se deconnecte, et -1 en cas d'erreur.
 */
int msg_send(int fd_circuitV, struct msg* m_send, int onlyContent) {

  int ret_send = 0, ret_shutdown = 0, ret_close = 0;
  int s_total_size = 0;
  void* sendS = (void*)m_send;
  unsigned int tailleMsg = m_send->size;

  // Si on ne doit envoyer que le content, on décale le pointeur
  if(onlyContent > 0){
    sendS = (void*)m_send->content;
  }

  // Tant que l'on a pas envoyé tout le message
  while (s_total_size < tailleMsg) {
    //printf("SEND :\n %d \n %d \n %s \n", m_send->size, m_send->cmd, m_send->content);
    ret_send = send(fd_circuitV, sendS + s_total_size, tailleMsg - s_total_size, 0);
    //printf("SEND OK (%d) \n", ret_send);
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


/* Recoit un message sur une socket
 *
 * @PARAM fd_circuitV, le descripteur de la socket
 * @PARAM m_send, la struct à recevoir
 * @PARAM onlyContent, 0 récéption du message complet (size, cmd, content)
 * > 0 récéption du content seulement
 * 
 * @RETURN le nombre d'octets reçus si tout se passe bien,
 * 0 si le pair se deconnecte, et -1 en cas d'erreur.
 */
int msg_recv(int fd_circuitV, struct msg* m_recv, int onlyContent) {

  int ret_recv = 0, ret_shutdown = 0, ret_close = 0;
  int r_total_size = 0;
  void* recvS = (void*)m_recv;
  unsigned int tailleMsg;

  // Si on ne doit recevoir que le content, on décale le pointeur
  if(onlyContent > 0){
    recvS = (void*)m_recv->content;
  }

  // Si c'est le premier message, on récupère la taille et la commande
  if(onlyContent == 0){

    while (r_total_size < sizeof(m_recv->size) + sizeof(m_recv->cmd)) {
      
      ret_recv = recv(fd_circuitV, recvS + r_total_size, sizeof(m_recv->size) + sizeof(m_recv->cmd) - r_total_size, 0);

      // S'il y a une erreur
      if(ret_recv == -1) {
        perror("recv size + cmd ");
        return ret_recv;
      }
      // Si la socket distante est fermée inopinement
      else if(ret_recv == 0) {

        ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
        if(ret_shutdown == -1) 
          perror("shutdown recv size + cmd ");

        // Ferme le descripteur de la socket
        ret_close = close(fd_circuitV);
        if(ret_close == -1)
          perror("close recv size + cmd ");

        return ret_recv;
      }

      r_total_size += ret_recv;
    }
    //printf("RECV SIZE + CMD :\n %d \n %d \n %d/%d \n", m_recv->size, m_recv->cmd, r_total_size, m_recv->size);
  }

  tailleMsg = m_recv->size;

  // Taille et commande du message reçu, maintenant récéption du contenu
  while(r_total_size < tailleMsg) {

    ret_recv = recv(fd_circuitV, recvS + r_total_size, tailleMsg - r_total_size, 0);

    // S'il y a une erreur
    if(ret_recv == -1) {
      perror("recv content ");
      return ret_recv;
    }
    // Si la socket distante est fermée inopinement
    else if(ret_recv == 0) {

      ret_shutdown = shutdown(fd_circuitV, SHUT_RDWR);
      if(ret_shutdown == -1)
        perror("shutdown recv content ");

      // Ferme le descripteur de la socket
      ret_close = close(fd_circuitV);
      if(ret_close == -1) 
        perror("close recv content ");

      return ret_recv;
    }

    r_total_size += ret_recv;
  }

  // Si le content n'est pas completement rempli, on place le \0 à la fin
  if(onlyContent == 0 && r_total_size < sizeof(*m_recv))
    m_recv->content[r_total_size - (sizeof(m_recv->size) + sizeof(m_recv->cmd))] = '\0';
  else if (onlyContent == 1 && r_total_size < sizeof(m_recv->content))
    m_recv->content[r_total_size] = '\0';
  
  //printf("RECV CONTENT :\n %s \n", m_recv->content);
  return r_total_size;
}