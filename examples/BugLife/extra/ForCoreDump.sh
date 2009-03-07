#!/bin/bash

#if you are getting bugs use this script to get a core dump for use with gdb

cd path/to/project
ulimit -c unlimited
./executable_name <params here> &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "executable_name(pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

