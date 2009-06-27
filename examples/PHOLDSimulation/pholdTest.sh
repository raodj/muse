#!/bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited
          #<X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
./phold    3  3  3    10       1          10 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "phold (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

