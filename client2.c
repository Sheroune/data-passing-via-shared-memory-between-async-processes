#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_SIZE 4096
const int PROJECT_ID = 'E';

struct sembuf plus2[1] = {{ 0, 2, 0 }};
struct sembuf min[1] = {{ 0, -1, 0 }};

struct semid_ds semid_ds;

union semun {
   int val;                // Value for SETVAL
   struct semid_ds *buf;   // Buffer for IPC_STAT, IPC_SET
   unsigned short *array;  // Array for GETALL, SETALL
   struct seminfo *__buf;  // Buffer for IPC_INFO (Linux specific)
} arg;

// Data structure in SHM
typedef struct mem_msg {
   int type;
   char buff[MAX_SIZE];
} message;

int main() {
   char buf[MAX_SIZE];

   key_t ipckey;
   message *msgptr;
   int shmid, semid;

   arg.buf = &semid_ds;

   memset(buf, 0, MAX_SIZE);

   // Get IPC key
   while(1){
      ipckey = ftok("/tmp/lab6", PROJECT_ID);
      if(ipckey == -1){
         printf("No messages here\n");
         sleep(1);
      }
      else
         break;
   }

   //Get or create SHM
   while(1){
      if((shmid = shmget(ipckey, 0, 0)) < 0) {
         printf("Shared memory connection error\nSleeping...\n");
            sleep(5);
      }
      else
         break;
   }

   // Get semaphores
   while(1){
      if((semid = semget(ipckey, 0, 0)) < 0) {
         printf("Semaphore get error\nSlepping...\n");
         sleep(4);
      }
      else
         break;
   }

   // Catch res
   printf("Waiting for resources\n");
   if(semop(semid, &min[0], 1) < 0) {
      printf("Catching resources error\n");
      exit(1);
   }

   // Connect shared memory
   if(((msgptr = (message*)shmat(shmid, 0, 0))) < 0) {
      printf("Shared memory connection error\n");
      exit(1);
   }

   printf("Sending message...");

   semctl(semid, 0, IPC_STAT, arg.buf);
   printf ("\nLast connection in %s\n", ctime(&arg.buf->sem_otime));
   // Write info to shm
   msgptr->type = 2;

   memset(msgptr->buff, 0, MAX_SIZE);
   strcpy(msgptr->buff, ctime(&arg.buf->sem_otime));

   // Disconnect shared memory
   if(shmdt(msgptr) < 0) {
      printf("Disconnection error\n");
      exit(1);
   }

   // free resources
   if(semop(semid, &plus2[0], 1) < 0) {
      printf("Free resources error\n");
      exit(1);
   }

   return 0;
}
