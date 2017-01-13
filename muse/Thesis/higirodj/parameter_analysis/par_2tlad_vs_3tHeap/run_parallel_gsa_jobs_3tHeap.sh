#!/bin/bash

# Just a convenience script that runs 5 different replications of
# gen_parallel_gsa_data.sh script as background jobs.

OUT_DIR="."
QUEUE2="3tHeap"

./gen_parallel_gsa_data.sh --reps 500 --skip 0 --queue2 $QUEUE2 --outfile "$OUT_DIR/par_gsa1.csv" >> "$OUT_DIR/par_gsa1_log.txt" 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 500 --queue2 $QUEUE2 --outfile "$OUT_DIR/par_gsa2.csv" >> "$OUT_DIR/par_gsa2_log.txt" 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 1000 --queue2 $QUEUE2 --outfile "$OUT_DIR/par_gsa3.csv" >> "$OUT_DIR/par_gsa3_log.txt" 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 1500 --queue2 $QUEUE2 --outfile "$OUT_DIR/par_gsa4.csv" >> "$OUT_DIR/par_gsa4_log.txt" 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 2000 --queue2 $QUEUE2 --outfile "$OUT_DIR/par_gsa5.csv" >> "$OUT_DIR/par_gsa5_log.txt" 2>&1 &

# End of script.
