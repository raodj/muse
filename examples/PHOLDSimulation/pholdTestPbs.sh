#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N phold_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=170GB
#PBS -l nodes=8:ppn=8
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
echo "time mpiexec ./phold 4000 4000 5 1 40 30 ..\n\n"
time mpiexec ./phold 4000 4000 5 1 40 30

#end of script

