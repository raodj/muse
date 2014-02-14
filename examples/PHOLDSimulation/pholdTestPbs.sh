#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N phold_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=120GB
#PBS -l nodes=2:ppn=4
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
echo "time mpiexec ./phold 100 100 5 1 8 30 ..\n\n"
# ulimit -c unlimited
time mpiexec -n 8 ./phold 100 100 5 1 8 30

#end of script

