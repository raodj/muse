#!/bin/bash

# A simple shell script to ease running callgrind to perform detailed
# profiling of phold executable with different granularity settings.

cd $PBS_O_WORKDIR
echo granularity = $GRANULARITY
time valgrind --tool=callgrind ./phold --rows 50 --cols 50 --granularity $GRANULARITY --simEndTime 1000 --scheduler-queue heap2tQ

# End of script
