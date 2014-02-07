#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N buglife_benchmark
#PBS -l walltime=1:00:00
#PBS -l mem=20GB
#PBS -l nodes=1:ppn=2
#PBS -S /bin/bash

cd $HOME/research/muse/examples/BugLife
#time ./bugsim 20 20 3 1 30
echo "time mpiexec ./bugsim --cols 10 --rows 10 --bugs 12 --nodes 4 --end 200 ..\n\n"
time mpiexec ./bugsim --cols 10 --rows 10 --bugs 12 --nodes 4 --end 200

#end of script

