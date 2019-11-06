#!/bin/bash
# Lines beginning with # are comments
# Only lines beginning #PBS are processed by qsub

#PBS -N $HOME/pholdLogFiles/phold_benchmark
#PBS -l walltime=1:00:00
#PBS -l mem=10GB
#PBS -l nodes=3:ppn=12
#PBS -S /bin/bash

cd $HOME/research/muse/examples/PHOLDSimulation
for n in 1 2 4 8 12 
do
  args="--rows 200 --cols 200 --eventsPerAgent 5 --delay 5 --simEndTime 500"
  execs="time mpiexec -n $n"
  >&2 echo -e "$execs ./phold $args \n"
  >&2 echo -e "\n #######################"
  echo -e "$execs ./phold $args \n"
  echo -e "\n #######################"
  for i in {1..10}
  do
    $execs ./phold $args
    >&2 echo -e "\n"
  done
  >&2 echo -e "\n\n"
done

#end of script
