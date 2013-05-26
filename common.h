//
//  common.h
//  ipc-play
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#ifndef ipc_play_Common_h
#define ipc_play_Common_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //input params
#include <time.h>
#include <unistd.h> //usleep
#include <sys/types.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

const int SEG_KEY = 1337;
const int SEG_KEY_READERS = 1338;
const int NB_ELEMENTS = 50;
const int NB_READERS = 2;
const int DATA_END = INT_MAX;
const char * SEM_READERS_MUTEX = "/ipc-play/readers-mutex";
const char * SEM_DATA_READY = "/ipc-play/data_free";
const char * SEM_DATA_READ = "/ipc-play/data_read";

#define TRACE(fmt, ...) \
    do { fprintf(stderr, "[PID %d]:: " fmt "\n", getpid(), __VA_ARGS__); } while (0)


#endif //ipc_play_Common_h


