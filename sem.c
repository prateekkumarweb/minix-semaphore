/**
 * Producer Consumer problem - Semaphore implementation
 * Author : Prateek Kumar
 * Email : prateek@prateekkumar.in
 */
#include <lib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// semaphore structure
typedef struct {
	int *value;
	int *size;
	int *head;
	int *tail;
	int *list;
	int shmid;
} semaphore;

// Array is used to represent circular queue
// enqueue into the process queue
void enqueue(semaphore *S, int p) {
	if (*S->head == (*S->tail+1)%*S->size) {
		return;
	}
	*S->tail = (*S->tail+1)%*S->size;
	S->list[*S->tail] = p;
	if (*S->head == -1)
		*S->head = 0;
}

// dequeue from the process queue
int dequeue(semaphore *S) {
	if (*S->head == -1 && *S->tail == -1) {
		return 0;
	}
	int r = S->list[*(S->head)];
	*S->head = *S->head + 1;
	if (*S->head == *S->size) 
		*S->head = 0;
	if (*S->head-1 == *S->tail) 
		*S->head = *S->tail = -1;
	return r;
}

// Initialize the semaphore
void* psem_init(semaphore *S, int n, int size, key_t key, int init) {
	message m;
	void* sem = NULL;
	if (init) {
		m.m1_i1 = 1;
		sem = _syscall(PM_PROC_NR, PSEM_INIT, &m);
	}
	// shared space is used for process queue
	int shmid;
	int *list;
	if (init) {
		if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) < 0) {
	        perror("shmget");
	        exit(1);
	    }	
	}
	else {
		if ((shmid = shmget(key, 1024, 0666)) < 0) {
	        perror("shmget");
	        exit(1);
	    }
	}
    if ((list = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // Initialize the semaphore values and return it
    S->value = &list[0];
    S->size = &list[1];
    S->head = &list[2];
    S->tail = &list[3];
	S->list = &list[4];
	S->shmid = shmid;
	if (init) {
		*S->value = n;
		*S->size = size;
		*S->head = *S->tail = -1;
	}
	return sem;
}

// Wait on semaphore
int psem_wait(semaphore *S, void* mtx) {
	// Mutex is used in order to ensure atomicity
	message m;
	m.m1_p1 = mtx;
	_syscall(PM_PROC_NR, PSEM_WAIT, &m);

	// Decrement the semaphore and log into th file
	(*S->value)--;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	FILE* fd = fopen("semLogfile", "a+");
	fprintf(fd, "Wait: Current value of S value %d at time: %lf\n", *S->value, tv.tv_sec*1e3+(double)tv.tv_usec/1000);
	
	if (*S->value < 0) {
		// Add the current process to queue and block it
		int pid = _syscall(PM_PROC_NR, PROC_NO, &m);
		enqueue(S, pid);
		
		gettimeofday(&tv, NULL);
		fprintf(fd, "Process to be blocked: %d at time: %lf\n", pid, tv.tv_sec*1e3+(double)tv.tv_usec/1000);
		fclose(fd);
		m.m1_p1 = mtx;
		_syscall(PM_PROC_NR, PSEM_BLOCK, &m);
	}
	else {
		fclose(fd);
		m.m1_p1 = mtx;
		_syscall(PM_PROC_NR, PSEM_SIGNAL, &m);
	}
	return 0;
}

// signal on semaphore
int psem_signal(semaphore *S, void* mtx) {
	// Mutex is used in order to ensure atomicity
	message m;
	m.m1_p1 = mtx;
	_syscall(PM_PROC_NR, PSEM_WAIT, &m);

	// Increment the semaphore and log into the file
	(*S->value)++;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	FILE *fd = fopen("semLogfile", "a+");
	fprintf(fd, "Signal: Current value of S value %d at time: %lf\n", *S->value, tv.tv_sec*1e3+(double)tv.tv_usec/1000);
	
	if (*S->value <= 0) {
		// Dequeue a process and wake up
		int proc_nr = dequeue(S);

		if (proc_nr != 0) {
			int pid = _syscall(PM_PROC_NR, PROC_NO, &m);
			gettimeofday(&tv, NULL);
			fprintf(fd, "Process to be woken up: %d by process: %d at time: %lf\n", proc_nr, pid, tv.tv_sec*1e3+(double)tv.tv_usec/1000);
			m.m1_i1 = proc_nr;
			_syscall(PM_PROC_NR, PSEM_WAKEUP, &m);
		}
	}
	fclose(fd);
	m.m1_p1 = mtx;
	_syscall(PM_PROC_NR, PSEM_SIGNAL, &m);
	return 0;
}

// Destroy the semaphore by deleting the shared data
void psem_destroy(semaphore *S) {
	shmdt(S->value);
	shmctl(S->shmid, IPC_RMID, 0);
}

// Get process id
int getpid() {
	message m;
	return _syscall(PM_PROC_NR, PROC_NO, &m);
}

// Generate a random number between 0 and 1
double rand_val()
{
	int seed = time(NULL);
    srand(seed);
	return (double)rand() / (double)RAND_MAX ;
}

// Return random number from exponential distribution
double expon(double x)
{
	double z = rand_val();
	double exp_value = -x * log(z);
	return(exp_value);
}
