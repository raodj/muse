#!/bin/bash

# This is a convenience script to submit 'n' parallel of PBS jobs to run
# sensitivity analysis using the gen_gsa_data.sh script.

SKIP=0
REPS=500
# SUB_DIR="./lad_vs_2tlad"
# SUB_DIR="./2tlad_vs_heap2tQ"
# SUB_DIR="./2tlad_vs_2tHeap"
# SUB_DIR="./2tlad_vs_heap"
# SUB_DIR="./2tlad_vs_fibHeap"
# SUB_DIR="./2tlad_vs_3tHeap_unifRecv"
# SUB_DIR="./2tlad_vs_3tHeap_unifDelay_unifRecv"
SUB_DIR="./heap_vs_binomHeap_expDelay_unifRecv"

# Compile Sobol random number generator
# g++ -g -Wall -std=c++11 Sobol.cpp -o Sobol

# Make a link to PHOLD for convenience
ln -s ${HOME}/research/muse/examples/PCS_Simulation/pcs .

for job in `seq 1 5`
do
    name="gsa_${SUB_DIR}_${job}"
    outFile="${SUB_DIR}/raw_gsa_data_${job}.csv"
    qsub -N $name -l"nodes=1:ppn=7" -l"mem=24gb" -l"walltime=8:00:00" -l"epilogue=${HOME}/epilogue.sh" -v "NUM_REPS=$REPS, SKIP_VALS=$SKIP, OUT_FILE=$outFile, QUEUE1=heap, QUEUE2=binomHeap" gen_gsa_data.sh
    # Update for next job
    SKIP=$(( SKIP + REPS ))
done

# End of script
