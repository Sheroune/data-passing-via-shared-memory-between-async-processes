#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_SIZE 4096
const int PROJECT_ID = 'E';

struct sembuf min[1] = {{ 0, -2, 0 }};
struct sembuf plus[1] = {{ 0, 1, 0 }};

typedef struct mem_msg {
   int type;
   char buff[MAX_SIZE];
} message;

union semun {
   int val;                // Value for SETVAL
   struct semid_ds *buf;   // Buffer for IPC_STAT, IPC_SET
   unsigned short *array;  // Array for GETALL, SETALL
   struct seminfo *__buf;  // Buffer for IPC_INFO (Linux specific)
} arg;

int main() {
   int shmid, semid;
   key_t ipckey;
   message* msgptr;

   arg.val = 1;

   system("touch /tmp/lab6");

   // get ipc key
   if ((ipckey = ftok("/tmp/lab6", PROJECT_ID)) < 0) {
      printf("Unable to get a key\n");
      exit(1);
   }

   // create shared memory
   if ((shmid = shmget(ipckey, sizeof(message), IPC_CREAT | 0666)) < 0) {
      printf("Unable to create ROP\n");
      exit(1);
   }

   // create semaphore
   if ((semid = semget(ipckey, 1, IPC_CREAT | 0666)) < 0) {
      printf("Unable to create semaphore\n");
      exit(1);
   }

   // set sem val to 1
   semctl(semid, 0, SETVAL, arg);


   while(1) {
      // wait for client resources
      printf("Waiting for resources\n");
      if (semop(semid, &min[0], 1) < 0) {
         printf("Error in waiting for resources\n");
         exit(1);
      }

      printf("Type 'q' to exit or 'ENTER' to continue\n");
      if(getchar()=='q')
         break;

      // connect shared memory
      if ((msgptr = (message*)shmat(shmid, 0, 0)) < 0) {
         printf("Error in connection of shm\n\n");
         exit(1);
      }

      printf("Message from client №%d. Content:\n\n%s\n", msgptr->type, msgptr->buff);

      memset(msgptr->buff, 0, MAX_SIZE);

      if(msgptr->type==1) {
         FILE* ppid = popen("ps -eo ppid,priority | awk '$2 > 15 {print $1}'", "r");
         fread(msgptr->buff, 1, sizeof(msgptr->buff), ppid);
         pclose(ppid);
      }

      // Disconnect shm
      if (shmdt(msgptr) < 0) {
         printf("Disconnection shm error\n");
         exit(1);
      }

      // free resources
      if (semop(semid, &plus[0], 1) < 0) {
         printf("Free resources error\n");
         exit(1);
      }
   }

   // remove shared memory
   if (shmctl(shmid, IPC_RMID, 0) < 0) {
      printf("Error removing shared memory\n");
      exit(1);
   }

   // remove semaphore
   if (semctl(semid, 0, IPC_RMID, 0) < 0) {
      printf("Error removing semaphore\n");
      exit(1);
   }

   return 0;
}
