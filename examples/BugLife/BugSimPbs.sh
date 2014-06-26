#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N buglife_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=200GB
#PBS -l nodes=2:ppn=4
#PBS -S /bin/bash

cd $HOME/research/muse/examples/BugLife
#ulimit -c unlimited
#echo "unlimit \n"
args="--cols 7000 --rows 7000 --bugs 22000000 --nodes 8 --end 700000"
echo -e "time mpiexec ./bugsim $args ..\n\n"
echo -e "\n ############################"
time mpiexec ./bugsim $args

#end of script
