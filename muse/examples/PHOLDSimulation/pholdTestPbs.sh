#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N $HOME/pholdLogFiles/phold_benchmark
#PBS -l walltime=5:00:00
#PBS -l mem=120GB
#PBS -l nodes=1:ppn=1
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
args="--rows 200 --cols 200 --eventsPerAgent 1000 --delay 5 --computeNodes 1 --simEndTime 100"
echo -e "time mpiexec ./phold $args \n"
echo -e "\n #######################"
# ulimit -c unlimited
time mpiexec ./phold $args

#end of script
