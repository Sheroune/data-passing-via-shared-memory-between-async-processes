#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>



#define MAX_SIZE 4096
const int PROJECT_ID = 'E';

typedef struct mem_msg {
   int segment;
   char buff[MAX_SIZE];
} message;

int main() {
   int shmid, semid;
   key_t ipckey;
   message* msgptr;

   system("touch /tmp/lab6");

   if ((ipckey = ftok("/tmp/lab6", PROJECT_ID)) < 0) {
        printf("Unable to get a key\n");
        exit(1);
    }

    if ((shmid = shmget(ipckey, sizeof(message), IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        printf("Unable to create ROP\n");
        exit(1);
    }


   return 0;
}
