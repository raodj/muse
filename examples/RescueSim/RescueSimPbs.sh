#!/bin/bash

#PBS -N RescueSim_benchmark
#PBS -l walltime=2:00:00
#PBS -l mem=20GB
#PBS -l nodes=1:ppn=2
#PBS -S /bin/bash

cd $HOME/research/muse/examples/RescueSim
echo "time mpiexec -n 2 ./RescueSim --cols 300 --rows 300 --vols 3000 --CCCx 150 --CCCy 150"
time mpiexec -n 2 ./RescueSim --cols 300 --rows 300 --vols 3000 --CCCx 150 --CCCy 150
mv RescueSim_benchmark.* tests/