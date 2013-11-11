#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N phold_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=40GB
#PBS -l nodes=2:ppn=2
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
time mpiexec ./phold 10 10 10 5 4 30

#end of script

