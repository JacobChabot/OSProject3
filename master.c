#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

// starting values for n and seconds
int n = 20;
int seconds = 100;

void help() {
	printf("Help function\n");
}



int main(int argc, char** argv) {
	
	// case and switch to handle command line arguments
	char ch;
	while ((ch = getopt(argc, argv, "hn:t:")) != -1) {
		switch (ch) {
			case 'h':
				help();
				break;
			case 't':
				sscanf(optarg, "%d", &seconds);
				continue;
			case 'n':					//i couldnt figure out to only accept n with no option
				sscanf(optarg, "%d", &n);
				if ( n > 20) {
					fprintf(stderr, "\nMax limit of n reached.\n");
					exit(0);
				}
				continue;
			default:
				fprintf(stderr, "Unrecononized options!\n");
				break;
			}
	}

	int processes[n];

	// initialize shared memory for turn variable
	int shm_id = shmget(2271999, sizeof(int), IPC_CREAT | 0666); //return identifier
	if (shm_id <= 0) {
		fprintf(stderr, "Share memory get failed\n");
		exit(1);
	}
	int * turn = shmat(shm_id, 0, 0); //attach pointer to shared memory identifier
	if (turn <= 0) {
		fprintf(stderr, "Shared memory attach failed\n");
		exit(1);
	}

	turn[0] = 10;

	time_t start, end, timer; 
	start = time(NULL);

	int i;
	for (i = 0; i < n; i++) {
		// convert i into a temp char to be able to pass as an argument to slave program
		char temp[10];
		snprintf(temp, sizeof(temp), "%d", i);
		
		// begin forking
		pid_t pid = fork();
		if (pid == 0) {
			// if child, execute slave program and exit
			execl("./slave", "./slave", temp, NULL);
			exit(0);
		}
		else {
			processes[i] = pid; //put the pid int he processes array
		}
		
		// calculate the timer, if timer is greater than seconds, loop through all child processes and kill them
		end = time(NULL);
		timer = end - start;
		if (timer > seconds) {
			int x;
			for (x = 0; x < i; x++) {
				kill(processes[x], SIGKILL);
			}
			break;			
		}

	}

	wait(0); // wait for child processes to finish

	//free memory
	shmdt(turn);
	shmctl(shm_id, IPC_RMID, NULL);


	
	printf("Child PIDs: ");
	for (i = 0; i < n; i++){
		printf("%d ", processes[i]);
	}

	printf("\nEnd of master.\n");

	return 0;
}
