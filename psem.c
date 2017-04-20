/**
 * Producer Consumer problem - Semaphore implementaion as syscall
 * Author : Prateek Kumar
 * Email : prateek@prateekkumar.in
 */
#include "pm.h"
#include "param.h"
#include "glo.h"
#include "mproc.h"
#include <lib.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// process node structure which will be used in linked list as queue
typedef struct process {
	int id;
	struct process *prev;
	struct process *next;
} process;

// enqueue into process queue
void enqueue(process **queue, process *p) {
	process* list = *queue;
	if (list == NULL) {
		*queue = p;
		p->prev = NULL;
		p->next = NULL;
		return;
	}
	while (list->next != NULL)
		list = list->next;
	list->next = p;
	p->prev = list;
	p->next = NULL;
}

// dequeue from process queue
process* dequeue(process **queue) {
	process* list = *queue;
	if (list == NULL)
		return NULL;
	*queue = list->next;
	if (list->next != NULL)
		list->next->prev = NULL;
	list->next = NULL;
	return list;
}


// semaphore structure
typedef struct {
	int value;
	process *list;
} semaphore;

// Initialize semaphore
void* do_psem_init() {
	int n = m_in.m1_i1;
	semaphore *S = (semaphore *)malloc(sizeof(semaphore));
	S->value = n;
	S->list = NULL;
	return (void *)S;
}

// Wait on semaphore
int do_psem_wait() {
	semaphore *S = (semaphore *) m_in.m1_p1;
	S->value--;
	if (S->value < 0) {
		process *P = (process *)malloc(sizeof(process));
		P->id = who_p;
		P->next = P->prev = NULL;
		enqueue(&S->list, P);
		return (SUSPEND);
	}
	return 0;
}

// Signal on semaphore
int do_psem_signal() {
	semaphore *S = (semaphore *) m_in.m1_p1;
	S->value++;
	if (S->value <= 0) {
		process* P = dequeue(&S->list);
		int proc_nr = P->id;
		free(P);
		register struct mproc *obj = &mproc[proc_nr];
		setreply(proc_nr, 0);
	}
	return 0;
}

// Destroy semapahore and free memory
int do_psem_destroy() {
	semaphore *S = (semaphore *) m_in.m1_p1;
	process *P = dequeue(&S->list);
	while (P != NULL) {
		free(P);
		P = dequeue(&S->list);
	}
	free(S);
	return 0;
}

// return process number
int do_proc_no() {
	return who_p;
}

// Signal the mutex and block the process
int do_psem_block() {
	do_psem_signal();
	return (SUSPEND);
}

// Wake up the process with given process number
int do_psem_wakeup() {
	int proc_nr = m_in.m1_i1;
	register struct mproc *obj = &mproc[proc_nr];
	setreply(proc_nr, 0);
	return 0;
}
