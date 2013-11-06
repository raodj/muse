#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N hello
#PBS -l walltime=4:00:00
#PBS -l mem=40GB
#PBS -l nodes=4:ppn=8
#PBS -S /bin/bash

cd /home/varadhsa/research/muse/examples/PHOLDSimulation/
time ./phold 800 800 100 5 20 30

#end of script

