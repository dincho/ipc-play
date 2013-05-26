//
//  ipc.c
//  ipc-play
//
//  Created by Dincho Todorov on 5/26/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include <stdlib.h> //exit
#include <stdio.h> //perror
#include <semaphore.h> //sem_open
#include <sys/shm.h> //shmget(), shmat()
#include "trace.h" //TRACE macro

sem_t* create_mutex(const char *name, int val)
{
    sem_t *mutex;
    
    TRACE("sem_open: creating semaphore %s", name);
    if((mutex = sem_open(name, O_CREAT, 0644, val)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    return mutex;
}

sem_t* get_mutex(const char *name)
{
    sem_t *mutex;
    
    TRACE("sem_open: openning semaphore %s", name);
    if((mutex = sem_open(name, 0)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    return mutex;
}

void* create_shm(key_t key, int *rshmid)
{    
    /*
     * Get the segment. Error if does not exists.
     */
    int shmid = shmget(key, sizeof(int), IPC_CREAT | IPC_R | IPC_W);
    if (shmid == -1) {
        perror("shmget failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmget: created segment with id: %d", shmid);
    
    /*
     * Now we attach the segment to our data space.
     */
    void *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    if (rshmid != NULL) {
        *rshmid = shmid;
    }
    
    return shm;
}

void* get_shm(key_t key)
{
    /*
     * Get the segment. Error if does not exists.
     */
    int shmid = shmget(key, sizeof(int), 0);
    if (shmid == -1) {
        perror("shmget failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmget: found segment with id: %d", shmid);
    
    /*
     * Now we attach the segment to our data space.
     */
    void *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    return shm;
}

void detach_shm(void *shm)
{
    shmdt(shm);
    TRACE("shmdt: dettached shared memory at address: %p", shm);
}

void remove_shm(int shmid, int exitOnError)
{
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        if (exitOnError) {
            exit(EXIT_FAILURE);
        }
    } else {
        TRACE("shmctl: removed segment with id: %d", shmid);
    }
}