#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run simulations in parallel.  Run as
# (with parameters to simulation at the end):
#
# $ ./generate_any_queue_stats.sh --procs 4 --outfile ph4_fibHeap_4proc_timings.csv --params '--rows 100 --cols 100 --scheduler-queue fibHeap --simEndTime 500' > ph4_fibHeap_4proc_log.txt 2>&1 &

# The number of reps we would like to average
reps=10

# These are generic statistics applicable for any MUSE simulation:
# Setup the list of values to be summarized from the data files.  The
# array contains the list of parameters whose information is to be
# summarized in the following format:
#   "param_name [1|p] [sum|max|any] col_with_stat search words"
#
RunTimeStatsInfo=(
	"runtime     p max 8   h:mm:ss or m:ss"
	"memory      p max 6   Maximum resident set size"
	"rollbacks   p sum 9   Total #rollbacks "
	"mpiMsgs     p sum 7   Total #MPI messages "
	"schedules   p sum 10  Total schedules "
	"commEvts    p sum 4   Total Committed Events"
)

# Include helper functions
source ./generate_stats.sh

# Let the main function do all the necessary work
main $*

# The following code is used for testing / validation of stats
# PROCS=1
# TMPDIR="tmp_4939"
# REPS=3

# initCmdLineParams
# printHeader
# recordAllStats
# saveOutputFiles "$TMPDIR/pbs_job.sh"

# end of script
