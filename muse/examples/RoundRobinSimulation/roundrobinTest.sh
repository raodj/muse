#!/bin/bash

cd /home/gebremr/research/RoundRobinSimulation
ulimit -c unlimited
./roundrobinTest 4 2 10 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "roundrobinTest (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

