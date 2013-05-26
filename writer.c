//
//  main.c
//  writer
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include "common.h"
#include <time.h>

/* GLOBAL VARIABLES SO WE CAN CLEANUP ON INTERUPT */

int shmid; //return value from shmget()
void *shm; //shared memory pointer
sem_t *mutex;
sem_t *data_ready;
sem_t *data_read;

void cleanup();
void termination_handler(int signum);

int main(int argc, const char * argv[])
{
    if((data_ready = sem_open(SEM_DATA_READY, O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    if((data_read = sem_open(SEM_DATA_READ, O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    /*
     * Create the shared segment. Error if already exists.
     */
    if ((shmid = shmget(SEG_KEY, sizeof(int), IPC_CREAT | IPC_EXCL | IPC_R | IPC_W)) == -1) {
        perror("shmget failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmget: created segment with id: %d", shmid);
    
    /*
     * Attach the segment to the data space.
     */
    void *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmat: attached at address: %p", shm);
    
    //handle CRTL-C
    signal(SIGINT, termination_handler);
    
    //seed the random generator
    srand((unsigned int)time(NULL));
    
    //cast shared memory to our data type
    int *data = (int *) shm;
    
    TRACE("%s", "generating data: ");
    for(int i = 0; i < NB_ELEMENTS; i++) {
        
        sem_wait(data_read);
            //critical section
            *data = rand() % 100 + 1; //[1-100]
        sem_post(data_ready);
        
        //easy results testing
        TRACE("%d ", *data);
        
        //sleep for 200ms
        usleep(200 * 1000);
    }
    
    //end marker
    sem_wait(data_read);
        *data = DATA_END;
    sem_post(data_ready);
    
    printf("\nwriter done\n");
    cleanup();
    return EXIT_SUCCESS;
}


void cleanup()
{
    //deallocate the segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmctl: removed segment with id: %d", shmid);
    
    //The semaphore name is removed immediately.
    //The semaphore is destroyed once all other processes that have the semaphore open close it.
    sem_close(data_ready);
    sem_close(data_read);
    sem_unlink(SEM_DATA_READY);
    sem_unlink(SEM_DATA_READ);
    
    TRACE("%s", "closed and unlinked data semaphore");
}

void termination_handler(int signum)
{
    TRACE("%s", "OK, quitting now ..");
    cleanup();
    exit(EXIT_SUCCESS);
}
