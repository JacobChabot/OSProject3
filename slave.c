#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

enum state{idle, want_in, in_cs};

// function to write to log file
void logFile(int i, char * status) {
	FILE * file;
	// create file name 
	char fileName[20];
	snprintf(fileName, sizeof(fileName), "logfile.%d", i);
	
	// open file
	file = fopen(fileName, "a");
	if (file == NULL)
		perror("Error: ");

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
		perror("Error: ");

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
        int j; // local to each process
	
	// initialize shared memory for turn and flags
        int shm_id = shmget(2271999, sizeof(int) + n*sizeof(enum state), IPC_CREAT | 0666); //return identifier
        int * turn = shmat(shm_id, 0, 0); //attach pointer to shared memory identifier
        enum state *flag = shmat(shm_id, 0, 0);
	if (flag <= 0 || turn <= 0) {
		fprintf(stderr, "Shared memory attach failed\n");
		exit(0);
	}

	srand(time(0));

	// critical section code
	int number;
	int x;
	for (x = 0; x < 5; x++) { // loop 5 times to enter critical section 5 times
		number = ((rand() % (3 - 1 + 1)) + 1); // generate a random number between 1 and 3	
		
		// critical section entry
		do {
			
			flag[i] = want_in;
			j = turn[0];
			while (j != i) {
				if (flag[j] != idle) // j = (flag[j] != idle) ? turn : (j + 1) % n; i just understood this easier and made it easier to debugg
				       j = turn[0];
				else {
					if (turn[0] + 1 == n) {
                        			j = (turn[0] + 1) % (n + 1);
                			}
                			else {
                        			j = (turn[0] + 1) % n;
                        			if (j == 0)
                                			j = j + 1;
                			}

				// kill switch
                        	if (i == n && x == 4) // if last process, and on last loop of for loop, break while loop
                                	break;
				}
				      	       
			}

			flag[i] = in_cs; //declare intention to enter cs

			for (j = 1; j >= n; j++)
				if ((j != i) && (flag[j] == in_cs))
					break;
		} while ((j >= n) || (turn[0] != i && flag[turn[0]] != idle));
		
		turn[0] = i; // set turn to self
		char enterStatus[] = "has entered the critical section at: ";
		logFile(i, enterStatus); // call log function to write to log file
		sleep(number);
		
		critical_section(i); // enter critical section
		
		char exitStatus[] = "has exited the critical section at: ";
		logFile(i, exitStatus);
		sleep(number);
		
		if (turn[0] + 1 == n) {
			j = (turn[0] + 1) % (n + 1);
              	}
                else {
			j = (turn[0] + 1) % n;
                        if (j == 0)
                        	j = j + 1;
                }
		
		while (flag[j] == idle) {
			if (turn[0] + 1 == n) {
                        	j = (turn[0] + 1) % (n + 1);
			}	
                	else {
                        	j = (turn[0] + 1) % n;
				if (j == 0) 
					j = j + 1;
			}
			
			// kill switch
			if (i == n && x == 4) // if last process, and on last loop of for loop, break while loop
				break;	
		
		}
		
		turn[0] = j;
		flag[i] = idle;
	}
	
	//free memory
        shmdt(turn);
        shmdt(flag);
        shmctl(shm_id, IPC_RMID, NULL);

	return 0;
}
