//
//  trace.h
//  ipc-play
//
//  Created by Dincho Todorov on 5/26/13.
//  Copyright (c) 2013 Dincho Todorov. All rights reserved.
//
#include <stdio.h> //stderr
#include <unistd.h> //getpid()

#ifndef ipc_play_trace_h
#define ipc_play_trace_h

#define TRACE(fmt, ...) \
    do { fprintf(stderr, "[PID %d]:: " fmt "\n", getpid(), __VA_ARGS__); } while (0)

#endif
