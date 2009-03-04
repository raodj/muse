#!/bin/bash

cd /home/gebremr/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited
          #<X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
./pholdTest 1  20  10    10       4                 10000 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "pholdTest (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

