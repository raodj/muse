#!/bin/bash

cd /home/gebremr/research/RollbackHeavySimulation
ulimit -c unlimited
#valgrind --gen-suppressions=all 
./rollbackTest 2 2 &
pid=$!
wait $pid
status=$?
if [ $status -ne 0 ]
then
    echo "rollbackTest (pid = $pid) exited with status $status"
    cp "/tmp/corefiles/core.$pid" .
fi

