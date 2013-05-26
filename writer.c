//
//  main.c
//  writer
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include <stdlib.h> //exit
#include <stdio.h> //perror
#include <time.h>
#include <unistd.h> //usleep
#include <signal.h>
#include <semaphore.h>
#include "common.h"
#include "trace.h"
#include "ipc.h"

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
    //global mutexes - writer is the owner
    data_ready = create_mutex(SEM_DATA_READY, 0);
    data_read = create_mutex(SEM_DATA_READ, 1);
    
    //global shared memory (data buffer)
    int *data = (int *) create_shm(SEG_KEY, &shmid);
    
    //handle CRTL-C
    signal(SIGINT, termination_handler);
    
    //seed the random generator
    srand((unsigned int)time(NULL));
    
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
    remove_shm(shmid, 1);
    
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
