#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N phold_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=120GB
#PBS -l nodes=2:ppn=4
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
echo "time mpiexec ./phold --rows 100 --cols 100 --eventsPerAgent 5 --delay 1 --computeNodes 8 --simEndTime 30 ..\n\n"
# ulimit -c unlimited
time mpiexec -n 8 ./phold --rows 100 --cols 100 --eventsPerAgent 5 --delay 1 --computeNodes 8 --simEndTime 30 

#end of script

