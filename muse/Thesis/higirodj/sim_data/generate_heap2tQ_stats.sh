#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run simulations in parallel.  Run as
# (with parameters to simulation at the end):
#
# $ ./generate_heap2tQ_stats.sh --procs 4 --outfile ph4_heap2tQ_4proc_timings.csv --params '--rows 100 --cols 100 --scheduler-queue heap2tQ --simEndTime 500' > ph4_heap2tQ_4proc_log.txt 2>&1 &

# The number of reps we would like to average
reps=10

# Setup the list of values to be summarized from the data files.  The
# array contains the list of parameters whose information is to be
# summarized in the following format:
#   "param_name [1|p] [sum|max|any] col_with_stat search words"
#
RunTimeStatsInfo=(
	"runtime     1 max 2   ^real "
	"memory      p max 6   Maximum resident set size"
	"rollbacks   p sum 9   Total #rollbacks "
	"mpiMsgs     p sum 7   Total #MPI messages "
	"schedules   p sum 10  Total schedules "
	"commEvts    p sum 4   Total Committed Events"
	"agentBkts   p sum 12  Average #buckets per agent"
	"schedBktSz  p sum 9   Average scheduled bucket size"
	"fixHeapComp p sum 13  Average fixHeap compares"
	"fixHeapOps  p sum 11  Average fixHeap compares"
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
