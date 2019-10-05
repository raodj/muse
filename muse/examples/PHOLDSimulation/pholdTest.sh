#!/bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation 
ulimit -c unlimited

time mpiexec -n 1 ./phold --rows 3  --cols 3  --eventsPerAgent 3 --delay 1 --simEndTime 100 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "phold (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

