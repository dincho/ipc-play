//
//  common.h
//  ipc-play
//
//  Created by Dincho Todorov on 4/28/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//

#include <limits.h> //INT_MAX

#ifndef ipc_play_Common_h
#define ipc_play_Common_h

const int SEG_KEY = 1337;
const int SEG_KEY_READERS = 1338;
const int NB_ELEMENTS = 50;
const int DATA_END = INT_MAX;
const char * SEM_READERS_MUTEX = "/ipc-play/readers-mutex";
const char * SEM_DATA_READY = "/ipc-play/data_ready";
const char * SEM_DATA_READ = "/ipc-play/data_read";

#endif //ipc_play_Common_h


