//
//  main.c
//  reader
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include <stdlib.h> //exit consants
#include <string.h> //strcmp()
#include <signal.h>
#include <semaphore.h>
#include "common.h"
#include "trace.h"
#include "ipc.h"

/* GLOBAL VARIABLES SO WE CAN CLEANUP ON INTERUPT */

int fd; //output file descriptor
int *data; //shared memory pointer
int shmid_readers;
sem_t *mutex;
sem_t *data_ready;
sem_t *data_read;

/* prototypes */
int parse_params(int argc, const char * argv[]);
int process_data(int even, int data);
void cleanup();
void termination_handler(int signum);

int main(int argc, const char * argv[])
{
    //odd or even reader
    int even = parse_params(argc, argv);

    //local (readers) mutex
    mutex = create_mutex(SEM_READERS_MUTEX, 1);
    
    //not owned, but create the mutexes if the writer is not running
    data_ready = create_mutex(SEM_DATA_READY, 0);
    data_read = create_mutex(SEM_DATA_READ, 1);
    
    //global shared memory (data buffer)
    data = (int *) create_shm(SEG_KEY, NULL);
    
    //local shared memory (between readers), used for turnstile
    int *cnt_readers = (int *) create_shm(SEG_KEY_READERS, &shmid_readers);

    //handle CRTL-C
    signal(SIGINT, termination_handler);

    /* MAIN LOOP */
    int n; //temp data storage
    
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
        n = *data;
        
        sem_wait(mutex);
            (*cnt_readers)--;
            //last one, free the shm
            if (*cnt_readers == 0) {
                sem_post(data_read);
            }
        sem_post(mutex);
        
        if(0 == process_data(even, n)) {
            break;
        }
    }
    
    TRACE("%s", "done. Quitting now");
    cleanup();
    return EXIT_SUCCESS;
}

int parse_params(int argc, const char * argv[])
{
    char *filename;
    int even;
    
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
    
    return even;
}

int process_data(int even, int data)
{
    if (data == DATA_END) { //exit marker
        return 0;
    }
    
    if ((even && !(data & 1))
        || (!even && (data & 1))
    ) {
        ssize_t written = write(fd, &data, sizeof(data));
        if (written != sizeof(data)) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    
    return 1;
}

void cleanup()
{    
    //not needed because shared mem is auto-detached on process exit
    //just for full scenario
    detach_shm(data);
    
    //close named semaphore so resources can be freed
    sem_close(mutex);
    sem_close(data_ready);
    sem_close(data_read);
    sem_unlink(SEM_READERS_MUTEX);
    
    TRACE("%s", "sem_close: closed named semaphores");
    
    close(fd);
    TRACE("closed fd: %d", fd);
    
    remove_shm(shmid_readers, 0);
}

void termination_handler(int signum)
{
    TRACE("%s", "OK, quitting now ...");
    cleanup();
    exit(EXIT_SUCCESS);
}

