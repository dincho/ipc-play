#!/bin/sh

#  run.sh
#  ipc-play
#
#  Created by Dincho Todorov on 5/25/13.
#  Copyright (c) 2013 Dincho Todorov. All rights reserved.

#./writer 2> writer.log &
./reader odd 2> odd.log &
./reader even 2> even.log &