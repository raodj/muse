#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N buglife_benchmark
#PBS -l walltime=4:00:00
#PBS -l mem=40GB
#PBS -l nodes=1:ppn=2
#PBS -S /bin/bash

cd $HOME/research/muse/examples/BugLife
time ./bugsim 20 20 6 2 30

#end of script

