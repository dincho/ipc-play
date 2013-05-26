//
//  ipc.h
//  ipc-play
//
//  Created by Dincho Todorov on 5/26/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include <semaphore.h> //sem_t

#ifndef ipc_play_ipc_h
#define ipc_play_ipc_h

sem_t* create_mutex(const char *name, int val);
sem_t* get_mutex(const char *name);
void* create_shm(key_t key, int *rshmid);
void* get_shm(key_t key);
void detach_shm(void *shm);
void remove_shm(int shmid, int exitOnError);

#endif
