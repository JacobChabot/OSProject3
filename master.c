#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> // semaphores
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
	printf("master program\n");
	// create semaphore set
	key_t key = 2271999;
	// semget(key to create semophore, number of semophores needed, permissions)
	int sem = semget(key, 1, 0600 | IPC_CREAT); // permissions? 
	if (sem == -1) {
		perror("semget");
		exit(0);
	} 

	// initialize semaphore to 1
	// 1 = CS is available, 0 = CS is in use
	struct sembuf initSem = {
		.sem_num = 0,
		.sem_op = 1, // set to 1 
		.sem_flg = 0
	};
	if (semop(sem, &initSem, 1) == -1) {
		perror("semop failed");
		exit(0);
	}

	//semop(semophore ID, pointer to an sembuf array[], number of arrays)

	//semctl(sem ID, semaphore number, command);
	

	time_t start, end, timer; 
	start = time(NULL);
	
	int i;
	for (i = 0; i <= 5; i++) {
		// convert i into a temp char to be able to pass as an argument to slave program
		char temp[10];
		char nTemp[10];
		snprintf(temp, sizeof(temp), "%d", i);
		snprintf(nTemp, sizeof(nTemp), "%d", n);

		printf("for loop in master\n");
	
		// begin forking
                pid_t pid = fork();
                if (pid == 0) {
                        // if child, execute slave program and exit
                        execl("./slave.out", "./slave.out", temp, nTemp, NULL);
			exit(0);
		} 
		else {
			processes[i] = pid; //put the pid int he processes array
		}
           

	}

	

	// loop through all the child processes and wait for them to finish
        for (i = 0; i < 5; i++) {
                wait(0);
        }

	semctl(sem, 0, IPC_RMID);

	end = time(NULL);              	
	timer = end - start;
	
	
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
