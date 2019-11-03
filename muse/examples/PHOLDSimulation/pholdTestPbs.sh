#!/bin/bash

#PBS -N $HOME/pholdLogFiles/phold_benchmark
#PBS -l walltime=1:00:00
#PBS -l mem=10GB
#PBS -l nodes=3:ppn=1
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation

args="--rows 200 --cols 200 --eventsPerAgent 1000 --delay 5 --simEndTime 100"

echo -e "time mpiexec ./phold $args \n"
echo -e "\n #######################"

time mpiexec ./phold $args

#end of script
