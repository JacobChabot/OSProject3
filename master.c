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
	printf("./master.out -n <number of processes>(required) -s <number of seconds>(optional)");

	exit(0);
}

// control c signal handler function
void catchctrlc (int signo) {
	char handmsg[] = "^C Received\n"; //output
	int msglen = sizeof(handmsg);
	write(STDERR_FILENO, handmsg, msglen);
}

int main(int argc, char** argv) {
	
	// set up signal handling
	struct sigaction act;
	act.sa_handler = catchctrlc;
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1))
		perror("Failed to set SIGINT to handle CTRL C");

	// first check if n was provided
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0)
			break;
	}
	if ( i == argc) {
		fprintf(stderr, "Number of processes is a required argument\n\n");
		exit(0); // terminate if n was not provided
	}

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
					fprintf(stderr, "Max number of processes allowed\n");
					n = 20; // changed from exitting to setting n back to 20 after reviewing Alina's comments from project 2 
				}
				continue;
			default:
				fprintf(stderr, "Unrecononized options!\n");
				break;
			}
	}

	// processes array in order to terminate processes if needed
	int processes[n];
	
	// create semaphore set
	key_t key = ftok("./master.c", 0); // create key using ftok
	int sem = semget(key, 1, 0600 | IPC_CREAT); // generate semaphore and check for errors 
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

	time_t start, end, timer; 
	start = time(NULL);
	
	for (i = 0; i < n; i++) {
		// convert i and n into temp chars to be able to pass as an argument to slave program
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
			processes[i] = pid; //put the pid in the processes array
		}
	}

	// loop through all the child processes and wait for them to finish
        for (i = 0; i < n; i++) {
		if (wait(0)) {
			// after child finishes, check if timer has exceded the max amount and terminate processes
			end = time(NULL);
			timer = end - start;
			if (timer > seconds) {
				printf("kill processes\n");
				int x;
				for (x = 0; x < i; x++) 
					kill(processes[x], SIGKILL);
				break;
			}
		}
        }

	//delete semaphore 
	semctl(sem, 0, IPC_RMID);

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
	
        fprintf(cstest, "\nMaster completed at %s\n", timeString); // output time of completion

        fclose(cstest);

	printf("\nEnd of master.\n");

	return 0;
}
