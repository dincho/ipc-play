//
//  main.c
//  reader
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include "common.h"

/* GLOBAL VARIABLES SO WE CAN CLEANUP ON INTERUPT */

int fd; //output file descriptor
void *shm; //shared memory pointer
int shmid_readers;
sem_t *mutex;
sem_t *data_ready;
sem_t *data_read;

void cleanup();
void termination_handler(int signum);

int main(int argc, const char * argv[])
{
    //odd or even reader
    int even;
    char *filename;
    
    if (argc > 1 && 0 == strcmp(argv[1], "even")) { //necheten
        even = 1;
        filename = "even.bin";
    } else {
        even = 0;
        filename = "odd.bin";
    }
    
    /* OUTPUT FILE INIT */
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* SHARED SEGMENTS INITIALIZATION */
    int shmid = shmget(SEG_KEY, sizeof(int), 0);
    if (shmid == -1) {
        perror("shmget failed"); //handle errno
        if (ENOENT == errno) {
            TRACE("%s", "shmget: you need to run writer first");
        }
        
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmget: found segment with id: %d", shmid);
    
    /*
     * Now we attach the segment to our data space.
     */
    shm = shmat(shmid, NULL, SHM_RND | SHM_RDONLY);
    if (shm == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmat: attached at address: %p", shm);
    
    /*
     * Get the segment. Error if does not exists.
     */
    shmid_readers = shmget(SEG_KEY_READERS, sizeof(int), IPC_CREAT | IPC_R | IPC_W);
    if (shmid_readers == -1) {
        perror("shmget failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmget: found segment with id: %d", shmid_readers);
    
    /*
     * Now we attach the segment to our data space.
     */
    void *shm_readers = shmat(shmid_readers, NULL, 0);
    if (shm_readers == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmat: attached at address: %p", shm_readers);
    
    /* SEMAPHORES INITIALIZATION */
    
    TRACE("sem_open: openning semaphore %s", SEM_READERS_MUTEX);
    if((mutex = sem_open(SEM_READERS_MUTEX, O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("sem_open: openning semaphore %s", SEM_DATA_READY);
    if((data_ready = sem_open(SEM_DATA_READY, 0)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    TRACE("sem_open: openning semaphore %s", SEM_DATA_READ);
    if((data_read = sem_open(SEM_DATA_READ, 0)) == SEM_FAILED) {
        perror("sem_open failed"); //handle errno
        exit(EXIT_FAILURE);
    }
    
    signal(SIGINT, termination_handler);

    /* MAIN LOOP */
    int n; //temp data storage
    int *cnt_readers = (int *) shm_readers;
    
    TRACE("%s", "reading data...");
    
    while (1) {
        sem_wait(mutex);
            (*cnt_readers)++;
            //first one, wait for free shm
            if (*cnt_readers == 1) {
                sem_wait(data_ready);
            }
        sem_post(mutex);
        
        //critical section for readers
        n = *((int *) shm);
        
        sem_wait(mutex);
            (*cnt_readers)--;
            //last one, free the shm
            if (*cnt_readers == 0) {
                sem_post(data_read);
            }
        sem_post(mutex);
        
        if (n == DATA_END) { //exit marker
            break;
        }
        
        //work on the data
        if ((even && !(n & 1))
            || (!even && (n & 1))
        ) {
            ssize_t written = write(fd, &n, sizeof(n));
            if (written != sizeof(n)) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    TRACE("%s", "done. Quitting now");
    cleanup();
    return EXIT_SUCCESS;
}

void cleanup()
{
    close(fd);
    
    //not needed because shared mem is auto-detached on process exit
    //just for full scenario
    shmdt(shm);
    TRACE("shmdt: dettached shared memory at address: %p", shm);
    
    //deallocate readers segment
    if (shmctl(shmid_readers, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(EXIT_FAILURE);
    }
    
    TRACE("shmctl: removed segment with id: %d", shmid_readers);
    
    //close named semaphore so resources can be freed
    sem_close(mutex);
    sem_close(data_ready);
    sem_close(data_read);
    sem_unlink(SEM_READERS_MUTEX);
    
    TRACE("%s", "sem_close: closed named semaphores");
}

void termination_handler(int signum)
{
    TRACE("%s", "OK, quitting now ...");
    cleanup();
    exit(EXIT_SUCCESS);
}

