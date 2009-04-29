#!/bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited
          #<X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
./phold    100  100  3    10       2          500 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "phold (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

