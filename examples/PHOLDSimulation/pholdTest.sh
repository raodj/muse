#!/bin/bash

cd /home/gebremr/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited
          #<X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
./phold    2  2  2    10       3          1000 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "phold (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

