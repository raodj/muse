#!/bin/bash

cd $(HOME)/research/muse/examples/LonelySimulation 
ulimit -c unlimited
./lonelyTest 100 2 100 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "lonelyTest (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

