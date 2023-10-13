#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/sem.h> // semaphores

// function to write to log file
void logFile(int i, char * status) {
	FILE * file;
	// create file name 
	char fileName[20];
	snprintf(fileName, sizeof(fileName), "logfile.%d", i);
	
	// open file
	file = fopen(fileName, "a");
	if (file == NULL)
		perror("logfile");

	// get time in HH:MM:SS fromat
        time_t currentTime;
	struct tm * timeInfo;
	char timeString[9];
	time(&currentTime);
	timeInfo = localtime(&currentTime);
	strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);

	// write to file
	fprintf(file, "Process %d %s %s\n", i, status, timeString);

	fclose(file);
}

// critical section function
void critical_section(int process) {
	printf("Critical Section\n");
	
	FILE * cstest;
	cstest = fopen("cstest", "a");
	if (cstest == NULL)
		perror("cstest file");

	// get time in HH:MM:SS format
	time_t currentTime;
        struct tm * timeInfo;
        char timeString[9];
        time(&currentTime);
        timeInfo = localtime(&currentTime);
        strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);

	fprintf(cstest, "%s File modified by process number %d\n", timeString, process);

	fclose(cstest);
}

int main(int argc, char* argv[]) {	
	
	int n = atoi(argv[2]); // number of processes
        const int i = atoi(argv[1]); // own process number
	char enterStatus[] = "has entered the critical section at: ";
	char exitStatus[] = "has exited the critical section at: ";

	// create semaphore set
        key_t key = ftok("./master.c", 0); //generate key using ftok
        int sem = semget(key, 1, 0600 | IPC_CREAT); // generate semaphore and check for errors
        if (sem == -1) {
                perror("semget failed");
                exit(0);
        }

	srand(time(0));

	// critical section code
	int number;
	int x;
	for (x = 0; x < 5; x++) { // loop 5 times to enter critical section 5 times
		number = ((rand() % (3 - 1 + 1)) + 1); // generate a random number between 1 and 3	
		 
		struct sembuf semWait = {
    			.sem_num = 0,
			.sem_op = -1,
			.sem_flg = 0
		};
		if (semop(sem, &semWait, 1) == -1) { // wait for CS to be open
			perror("semop failed");
			exit(0);
		}
		
		// now in CS

		sleep(number);
		logFile(i, enterStatus); // write to logfile entered CS

		critical_section(i); // execute function to write to file while in CS
				
		sleep(number);

		struct sembuf semSignal = {
    			.sem_num = 0,
    			.sem_op = 1,
    			.sem_flg = 0
		};
		if (semop(sem, &semSignal, 1) == -1) { // signal that the CS can be opened
                	perror("semop failed");
                	exit(0);
        	}
	
		logFile(i, exitStatus); // write to log file exited CS

	}

	return 0;
}
