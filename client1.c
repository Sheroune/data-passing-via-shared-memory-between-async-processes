#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 4096
const int PROJECT_ID = 'E';

struct sembuf plus2 = { 0, 2, 0 };
struct sembuf plus = { 0, 1, 0 };
struct sembuf min = { 0, -1, 0 };

int main() {
   // Data structure in shared memory
   typedef struct mem_msg {
      int type;
      char buff[MAX_SIZE];
   } message;

   key_t ipckey;
   message *msgptr;
   int shmid, semid;

   // Get IPC key
   while(1){
      ipckey = ftok("/tmp/lab6", PROJECT_ID);
      if(ipckey == -1){
         printf("No messages\n");
         sleep(1);
      }
      else
         break;
   }

   // Get or create shared memory
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
         printf("Getting semaphore error\nSlepping...\n");
         sleep(5);
      }
      else
         break;
   }

   // Catch res
   printf("Waiting for resources\n");
   if(semop(semid, &min, 1) < 0) {
      printf("Catching resources error\n");
      exit(1);
   }

   // Connect shared memory
   if(((msgptr = (message*)shmat(shmid, 0, 0))) < 0) {
      printf("Shared memory connection error\n");
      exit(1);
   }

   printf("Sending message\n");

   // Write info to shared memory
   msgptr->type = 1;
   memset(msgptr->buff, 0, MAX_SIZE);

   FILE* shell = popen("ps -eo pid,time,priority | awk '$3 > 15 {print $1,$2}'", "r");
   fread(msgptr->buff, 1, sizeof(msgptr->buff), shell);
   pclose(shell);

   // Disconnect shared memory
   if(shmdt(msgptr) < 0) {
      printf("Shared memory disconnection error\n");
      exit(1);
   }

   // Free resources
   if(semop(semid, &plus2, 1) < 0) {
      printf("Free resources error\n");
      exit(1);
   }

   // Catch res
   printf("Wait for resources\n");
   if(semop(semid, &min, 1) < 0) {
      printf("Catching resources error\n");
      exit(1);
   }

   // Connect shared memory
   if(((msgptr = (message*)shmat(shmid, 0, 0))) < 0) {
      printf("Shared memory connection error\n");
      exit(1);
   }

   printf("Resourses were got\n");
   printf("Info from server:\n %s", msgptr->buff);

   // Disconnect shared memory
   if(shmdt(msgptr) < 0) {
      printf("Shared memory disconnection error\n");
      exit(1);
   }

   // Free resources
   if(semop(semid, &plus, 1) < 0) {
      printf("Free resources error\n");
      exit(1);
   }

   return 0;
}
