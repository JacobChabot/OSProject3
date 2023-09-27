#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char* argv[]) {
	
	// initialize shared memory for turn variable
        int shm_id = shmget(2271999, sizeof(int), IPC_CREAT | 0666); //return identifier
        int * turn = shmat(shm_id, 0, 0); //attach pointer to shared memory identifier


	int j = atoi(argv[1]); // local process number
	sleep(2);
	printf("Slave program: %d with %d\n", j, turn[0]);
	//printf("%s %d\n", argv[1], j);
	return 0;
}
