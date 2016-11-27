#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pwd.h>
#include <netdb.h>

#include <signal.h>
#include <ctype.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <pthread.h>
#include <sys/syscall.h>

#include <dirent.h>

// CLIENT CONSTANTS
#define MAX_FILE_SIZE 2048
#define SIZE_BUF_CMD 2048

// COMMANDS
#define BEGIN 0
#define GETLIST 1
#define GET 2

struct f_list {
  int file_nb;
  char** list;
};

struct msg {
  int msg_size; // taille du msg entier
  int cmd;
  union {
    char file_buffer [MAX_FILE_SIZE];
    struct f_list file_list;
  } content;
};


char* lstattoa(char* path_to_file);
struct f_list* listdir(char* path_to_dir);
