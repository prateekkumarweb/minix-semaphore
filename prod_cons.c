/**
 * Producer Consumer problem
 * Author : Prateek Kumar
 * Email : prateek@prateekkumar.in
 */
#include <lib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#include "sem.c"

int main(int argc, char *argv[]) {

	if (argc != 8) {
		printf("Usage:\n\t$ %s capacity np nc cntp cntc up uc\n", argv[0]);
		return 0;
	}

	// Take input from user capacity, np, nc, cntp, cntc, up, uc from command line arguements
	int capacity, np, nc, cntp, cntc, up, uc;
	capacity = atoi(argv[1]);
	np = atoi(argv[2]);
	nc = atoi(argv[3]);
	cntp = atoi(argv[4]);
	cntc = atoi(argv[5]);
	up = atoi(argv[6]);
	uc = atoi(argv[7]);

	// Open the files semLogfile, pcLogfile, timep.txt, timec.txt and make them empty
	FILE *fd;
	fd = fopen("semLogfile", "w+"); // Semaphore logs
	fprintf(fd, "");
	fclose(fd);

	fd = fopen("pcLogfile", "w+"); // Producer consumer logs
	fprintf(fd, "");
	fclose(fd);

	fd = fopen("timep.txt", "w+"); // time elapsed by producer processes
	fprintf(fd, "");
	fclose(fd);

	fd = fopen("timec.txt", "w+"); // time elapsed by consumer processes
	fprintf(fd, "");
	fclose(fd);
	
	// Shared semaphores which work as locks
	key_t key = 9876;
	int shmid;
	void **shm;
	if ((shmid = shmget(key, 50, IPC_CREAT | 0666)) < 0) {
       perror("shmget");
       exit(1);
   	}
   	if ((shm = shmat(shmid, NULL, 0)) == (void **) -1) {
       perror("shmat");
       exit(1);
   	}

   	// Shared buffer where producer produces into and consumer consumes from
	int *buffer;
	int shmid2;
	key_t key2 = 9875;
	if ((shmid2 = shmget(key2, 200*sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((buffer = shmat(shmid2, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // Initiaize in and out of buffer to 0
	buffer[0] = buffer[1] = 0;

	// Convert arguments so as to send as command line arguments
	int pid;
	char cap_str[10];
	char cntp_str[10];
	char cntc_str[10];
	char up_str[10];
	char uc_str[10];
	char tot_str[10];

	sprintf(cap_str, "%d", capacity);
	sprintf(cntp_str, "%d", cntp);
	sprintf(cntc_str, "%d", cntc);
	sprintf(up_str, "%d", up);
	sprintf(uc_str, "%d", uc);
	sprintf(tot_str, "%d", nc+np);

	// Mutex, empty and full semaphores that will be needed in producer consumer problem
	semaphore sem_m, sem_e, sem_f;
	shm[0] = psem_init(&sem_m, 1, np+nc, 9877, 1);
	shm[1] = psem_init(&sem_e, capacity, np+nc, 9878, 1);
	shm[2] = psem_init(&sem_f, 0, np+nc, 9879, 1);

	// Semaphores for mutex while writing in files 
	message m;
	m.m1_i1 = 1;
	shm[3] = _syscall(PM_PROC_NR, PSEM_INIT, &m);
	m.m1_i1 = 1;
	shm[4] = _syscall(PM_PROC_NR, PSEM_INIT, &m);
	m.m1_i1 = 1;
	shm[5] = _syscall(PM_PROC_NR, PSEM_INIT, &m);

	// Producer processes
	for (int i=0; i<np; i++) {
		pid = fork();
		if (pid == 0) {
			execlp("./producer", "producer", cap_str, cntp_str, tot_str, up_str, (char *)NULL);
		}
	}

	// Consumer processes
	for (int i=0; i<nc; i++) {
		pid = fork();
		if (pid == 0) {
			execlp("./consumer", "consumer", cap_str, cntc_str, tot_str, uc_str, (char *)NULL);
		}
	}

	// Wait for all producer and consumer processes to complete
	for (int i=0; i<nc+np; i++) {
		wait(NULL);
	}

	// Destroy the semaphores
	psem_destroy(&sem_m);
	psem_destroy(&sem_e);
	psem_destroy(&sem_f);
	m.m1_p1 = shm[0];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	m.m1_p1 = shm[1];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	m.m1_p1 = shm[2];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	m.m1_p1 = shm[3];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	m.m1_p1 = shm[4];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	m.m1_p1 = shm[5];
	_syscall(PM_PROC_NR, PSEM_DESTROY, &m);
	// Delete the shared memory segments
	shmdt(shm);
	shmctl(shmid, IPC_RMID, 0);
	shmdt(buffer);
	shmctl(shmid2, IPC_RMID, 0);
	return 0;
}
