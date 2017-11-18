#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run simulations in parallel.  Run as
# (with parameters to simulation at the end):
#
# $ ./generate_ladderq_stats.sh --procs 4 --outfile ph4_ladderQ_4proc_timings.csv --params '--rows 100 --cols 100 --scheduler-queue ladderQ --simEndTime 500' > ph4_ladderQ_4proc_log.txt 2>&1 &

# The number of reps we would like to average
reps=10

# Setup the list of values to be summarized from the data files.  The
# array contains the list of parameters whose information is to be
# summarized in the following format:
#   "param_name [1|p] [sum|max|any] col_with_stat search words"
#
RunTimeStatsInfo=(
	"runtime    p max 8   h:mm:ss or m:ss"
	"memory     p max 6   Maximum resident set size"
	"rollbacks  p sum 9   Total #rollbacks "
	"mpiMsgs    p sum 7   Total #MPI messages "
	"topCancl   p sum 12  Events cancelled from top"
	"ladCancl   p sum 9   Events cancelled from ladder"
	"botCancl   p sum 9   Events cancelled from bottom"
	"topScan    p sum 16  Events scanned in top"
	"ladScan    p sum 11  Events scanned from ladder"
	"botScan    p sum 11  Events scanned from bottom"
	"NuBotScan  p sum 9   Events scanned from bottom"
	"NoCBotSc   p sum 13  No cancel scans of bottom"
	"NuNoCBotSc p sum 11  No cancel scans of bottom"	
	"topIns     p sum 16  Inserts into top"
	"ladIns     p sum 14  Inserts into rungs"
	"botIns     p sum 13  Inserts into bottom"
	"rungs      p max 18  Max rung count"
	"bktRung    p sum 12  Average #buckets per rung"
	"botSize    p sum 17  Average bottom size"
	"bktSize    p sum 16  Average bucket width"
	"bot2Lad    p sum 10  Bottom to rung operations"
)

# Include helper functions
source ./generate_stats.sh

# Let the main function do all the necessary work
main $*

# The following code is used for testing / validation of stats
# PROCS=2
# TMPDIR="tmp_8520"
# REPS=10

# initCmdLineParams
# printHeader
# recordAllStats
# saveOutputFiles "$TMPDIR/pbs_job.sh"

# end of script
