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
	printf("Standard input: \n\n");
	printf("./master.out -n <number of processes>(optional) -s <number of seconds>(optional)");
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
	enum state {idle, want_in, in_cs};

	// initialize shared memory for turn variable
	int shm_id = shmget(2271999, sizeof(int) + n*sizeof(enum state), IPC_CREAT | 0666); //return identifier
	if (shm_id <= 0) {
		fprintf(stderr, "Share memory get failed\n");
		exit(1);
	}
	void * shmem = shmat(shm_id, 0, 0); //attach pointer to shared memory identifier
	if ((int*) shmem == -1) {
                perror("Error: ");
                exit(1);
        }
	int * turn = (int *) shmem;
	enum state * flag = (enum state *) (shmem + sizeof(int)); 

	int i; // initialize all the process flags to idle starting at 1
	for (i = 1; i < n; i++) {
		flag[i] = idle;
	}

	*turn = 0; // set turn to the first process

	time_t start, end, timer; 
	start = time(NULL);

	for (i = 1; i <= n; i++) {
		// convert i into a temp char to be able to pass as an argument to slave program
		char temp[10];
		char nTemp[10];
		snprintf(temp, sizeof(temp), "%d", i);
		snprintf(nTemp, sizeof(nTemp), "%d", n);
		
		// begin forking
		pid_t pid = fork();
		if (pid == 0) {
			// if child, execute slave program and exit
			execl("./slave.out", "./slave.out", temp, nTemp, NULL);
			exit(0);
		}
		else {
			processes[i-1] = pid; //put the pid int he processes array
		}

		// on last loop, calculate timer until and begin looping
               // if (i == n) {
		//	end = time(NULL);
                //	timer = end - start;
		//	while (timer < seconds) { // keep looping until timer > seconds, then begin killing processes
		//		sleep(5);
		//		end = time(NULL);
		//		timer = end - start;
		//		if (timer > seconds) {
		//			int x;
                  //      		for (x = 0; x < i; x++) {
		//				kill(processes[x], SIGKILL);
                  //      		}
		//			break;
                //		}
		//	}
                //}
	}

	// loop through all the child processes and wait for them to finish
	for (i = 0; i < n; i++) {
		int status;
		waitpid(processes[i], &status, 0);
	}

	end = time(NULL);              	
	timer = end - start;
	while (timer < seconds) { // loop until timer > seconds, then begin killing processes
		sleep(5);                  
		end = time(NULL);
		timer = end - start;
		if (timer > seconds) {
			int x;
			for (x = 0; x < i; x++) 
				kill(processes[x], SIGKILL);
			break;
		}	
	}
	
	//free memory
	shmdt(turn);
	shmdt(flag);
	shmctl(shm_id, IPC_RMID, NULL);
	
	// open cstest file
	FILE * cstest;
        cstest = fopen("cstest", "a");
        if (cstest == NULL)
                perror("Error: ");

        // get time in HH:MM:SS format
        time_t currentTime;
        struct tm * timeInfo;
        char timeString[9];
        time(&currentTime);
        timeInfo = localtime(&currentTime);
        strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);
	
        fprintf(cstest, "\nMaster completed at %s\n", timeString); // output timeof completion

        fclose(cstest);

	printf("\nEnd of master.\n");

	return 0;
}
