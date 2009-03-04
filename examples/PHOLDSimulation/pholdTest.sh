#!/bin/bash

cd /home/gebremr/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited
          #<X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
./pholdTest 10   2   10     2       1                 10000 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "pholdTest (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

