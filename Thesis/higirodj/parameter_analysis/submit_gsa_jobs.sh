#!/bin/bash

# This is a convenience script to submit 5 parallel of PBS jobs to run
# sensitivity analysis using the gen_gsa_data.sh script.

SKIP=1000
REPS=10

# Compile Sobol random number generator
g++ -g -Wall -std=c++11 Sobol.cpp -o Sobol

# Make a link to PHOLD for convenience
ln -s ${HOME}/research/muse/examples/PHOLDSimulation/phold .

for job in `seq 1 2`
do
    name="gsa_${job}"
    outFile="raw_gsa_data_${job}.csv"
    qsub -N $name -l"nodes=1:ppn=7" -l"mem=16gb" -l"walltime=6:00:00" -l"epilogue=${HOME}/epilogue.sh" -v "NUM_REPS=$REPS, SKIP_VALS=$SKIP, OUT_FILE=$outFile" gen_gsa_data.sh
    # Update for next job
    SKIP=$(( SKIP + REPS ))
done

# End of script
