#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>



#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>


#include "minimal-ws-server.c"

#define DEVICE_ADDRESS
#define DEVICE_MEMORY  "/sys/class/fe_dma/fe_dma0/memory"

struct wait_arg_struct {
  int * continueWait;
  int * ready;
};

extern int interrupted;
int active;

void* persistentReadRAM();
void* wait(void * arguments);
int readRAM(int addr, char buf[], int len);
void initializeRAM();
int socket_connect(char *host, in_port_t port);
void * connect_to_deployment_manager();

static char f[16];

int main(int argc, const char **argv) {
  pthread_t thread_id, dm_connection_thread;
  const int readLen = 16;
  char e[readLen];
  
  initializeRAM();
  signal(SIGINT, sigint_handler);

  pthread_create(&thread_id, NULL, &persistentReadRAM, NULL);

  attach_message_receive_callback(&send_message_all_clients);
  start_socket(argc, argv);

  while(interrupted == 0);

  return 0;
}

void* persistentReadRAM(){
  pthread_t thread_id;
  struct timeval time2;
  struct timeval t0;
  int * status, *readNow;
  unsigned int i = 0, n = 0;
  // float timeTemp;
  int val, len;

  struct wait_arg_struct wait_args;
  const int readLen = 16;
  char e[readLen];

  status = &interrupted;
  wait_args.continueWait = status;
  wait_args.ready = readNow;

  pthread_create(&thread_id, NULL, &wait, (void *) &wait_args);

  do{
    if(*readNow) {
      *readNow = 0;
      len = readRAM(n++, f, readLen);
      
      if(active) {
        send_message_all_clients(f, len);
        
      }
      if(i++ == 1000){
        i  = 0;
      }
      if(n == 256){
        n = 0;
      }
    }
    usleep(50);
  } while(*status == 0);

  return NULL;
}

void* wait(void * arguments) {
  int *continueWait, *ready;
  unsigned int i;
  struct wait_arg_struct *args = (struct wait_arg_struct *) arguments;
  continueWait = args->continueWait;
  ready = args->ready;

  do {
    usleep(1000 * 1000);
    *ready = 1;
  } while(*continueWait == 0);

  return NULL;
}

int readRAM(int addr, char buf[], int len) {
  struct timeval t0, t1;
  unsigned int i;
  int size;
  char addr_c[8];
  int mem_fd, addr_fd;

  #ifdef DEVICE_ADDRESS
  sprintf(addr_c, "%d", addr);
  if( ((addr_fd = open( DEVICE_ADDRESS, ( O_WRONLY) )) == -1) ) {
      printf( "ERROR: could not open \""DEVICE_ADDRESS"\"...\n" );
      return -1;
    }

  #endif

  if( ((mem_fd = open( DEVICE_MEMORY, ( O_RDONLY) ))  == -1) ) {
      printf( "ERROR: could not open \""DEVICE_MEMORY"\"...\n" );
      return -1;
    }
  #ifdef DEVICE_ADDRESS
  write(addr_fd, addr_c, 8);
  close(addr_fd);
  #endif
  
  size = read(mem_fd, buf, len);
  if(size < len) {
    buf[size] = '\0';
  }
  else {
    buf[len - 1] = '\0';
  }
   close(mem_fd);

  return size;
}
#ifdef DEVICE_ADDRESS
void initializeRAM() {
  unsigned int i;
  int fd, addr_fd;
  
  if( ( fd = open( DEVICE_MEMORY, ( O_WRONLY) ) ) == -1 ) {
      printf( "ERROR: could not open to write \""DEVICE_MEMORY"\"...\n" );
      return;
    }
  printf("Opened the device!\n");

  char d[1536];
  for(i = 0; i < 512; i++){
    sprintf(d, "%u %u", i, i);
    write(fd, &d, 1536);
  }

  close(fd);
  return;
}
#else 
void initializeRAM() {}
#endif