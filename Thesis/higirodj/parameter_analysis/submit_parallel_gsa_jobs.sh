#!/bin/bash

# Just a convenience script that runs 5 different replications of
# gen_parallel_gsa_data.sh script as background jobs.

./gen_parallel_gsa_data.sh --reps 500 --skip 324 --outfile par_gsa1.csv >> par_gsa1_log.txt 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 1085 --outfile par_gsa2.csv >> par_gsa2_log.txt 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 1583 --outfile par_gsa3.csv >> par_gsa3_log.txt 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 2086 --outfile par_gsa4.csv >> par_gsa4_log.txt 2>&1 &
./gen_parallel_gsa_data.sh --reps 500 --skip 2585 --outfile par_gsa5.csv >> par_gsa5_log.txt 2>&1 &

# End of script.
