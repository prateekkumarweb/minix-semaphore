/**
 * Producer Consumer problem - Consumer program
 * Author : Prateek Kumar
 * Email : prateek@prateekkumar.in
 */
#include <lib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sem.c"

int main(int argc, char* argv[]) {

	time_t start, end;
	double runtime;

	time(&start);

	int bufsize = atoi(argv[1]); // Buffer size
	int cnt = atoi(argv[2]); // Number of consumer processes
	int tot = atoi(argv[3]); // Total number of processes
	int mean = atoi(argv[4]); // Mean of time of sleep

	// Initialize the semaphores
	semaphore sem_m, sem_e, sem_f;
	psem_init(&sem_m, 1, tot, 9877, 0);
	psem_init(&sem_e, bufsize, tot, 9878, 0);
	psem_init(&sem_f, 0, tot, 9879, 0);

	// Shared memory for other semaphores that will be used while writing to files
	key_t key = 9876;
	int shmid;
	void **shm;
	if ((shmid = shmget(key, 50, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((shm = shmat(shmid, NULL, 0)) == (void **) -1) {
        perror("shmat");
        exit(1);
    }

    // Shared memory for the buffer
	key_t key2 = 9875;
	int *shm2;
	if ((shmid = shmget(key2, 200*sizeof(int), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((shm2 = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

	int* out = &shm2[1];
	int* buffer = &shm2[2];
	message m;

	// pid of the process
	int pid = getpid();

	struct timeval tv;


	for (int i=0; i<cnt; i++) {
		psem_wait(&sem_f, shm[2]); // wait on full sem
		psem_wait(&sem_m, shm[0]); // wait on mutex sem

		// remove from buffer
		int data = buffer[*out];
		int prodat = *out;
		*out = (*out + 1 )%bufsize;
		gettimeofday(&tv, NULL);

		// Log ith item: item read from the buffer by the process pid at consTime from location m
		// Lock the file before logging
		message m;
		m.m1_p1 = shm[3];
		_syscall(PM_PROC_NR, PSEM_WAIT, &m);

		FILE *fd = fopen("pcLogfile", "a+");
		fprintf(fd, "%dth item: %d consumed by process %d at %lf from buffer location %d\n", i, buffer[prodat], pid, tv.tv_sec*1e3+(double)tv.tv_usec/1000, prodat);
		fclose(fd);

		m.m1_p1 = shm[3];
		_syscall(PM_PROC_NR, PSEM_SIGNAL, &m);
		
		psem_signal(&sem_m, shm[0]); // signal mutex sem
		psem_signal(&sem_e, shm[1]); // signal empty sem
		//consume	
		// sleep
		double t = expon(mean);
		sleep(t);
	}

	// After process completes write the time elapsed to the files

	time(&end);

	runtime = ((double)(end-start));
	runtime /= cnt;

	m.m1_p1 = shm[5];
	_syscall(PM_PROC_NR, PSEM_WAIT, &m);

	FILE *fd = fopen("timec.txt", "a+");
	fprintf(fd, "%lf\n", runtime);
	fclose(fd);

	m.m1_p1 = shm[5];
	_syscall(PM_PROC_NR, PSEM_SIGNAL, &m);
}
