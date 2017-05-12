#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N $HOME/pcsLogFiles/pcs_benchmark
#PBS -l walltime=1:00:00
#PBS -l mem=4GB
#PBS -l nodes=1:ppn=1
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PCS_Simulation
args="--rows 95 --cols 96 --eventsPerAgent 19 --moveIntervalMean 4 --callIntervalMean 9 --callDurationMean 9 --maxChannels 91  --imbalance 0.280 --delay 1 --granularity 1 --selfEvents 0.84 --scheduler-queue 2tLadderQ --simEndTime 374"
echo -e "time mpiexec ./pcs $args \n"
echo -e "\n #######################"
# ulimit -c unlimited
time mpiexec ./pcs $args

#end of script
