#!/bin/bash

# Setup the different statistics we would like for the helper scripts
# to record for the different scheduler-queue types.

#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run multiple parallel simulations.  Run
# as (with parameters to simulation at the end):
#
# $ ./generate_parallel_stats.sh --procs "2,4,8,16,32,64,128" --outfile ph4_ladderQ_parallel_timings.csv --params '--delay 2 --selfEvents% 25 --eventsPerAgent 2 --rows 100 --cols 100 --scheduler-queue ladderQ --simEndTime 500' > ph4_ladderQ_parallel_log.txt 2>&1 &

# Include helper functions
source ./generate_stats.sh

# Convenience function to setup the array with runtime statistics to
# collect for the different queues based on the queue name specified
# in PARAMS. The list of values to be summarized from the data files.
# The array contains the list of parameters whose information is to be
# summarized in the following format:
#
#   "param_name [1|p] [sum|max|any] col_with_stat search words"
#
# NOTE: The PARAMS variable is set in the included helper scripts
#
function setupRuntimeStats() {
	if [[ $SIM_PARAMS == *"ladderQ"* ]]; then
		echo "** Gathering ladderQ runtime stats **"
		RunTimeStatsInfo=(
			"runtime   1 max 2   ^real "
			"memory    p max 6   Maximum resident set size"
			"rollbacks p sum 9   Total #rollbacks "
			"mpiMsgs   p sum 7   Total #MPI messages "
			"topCancl  p sum 12  Events cancelled from top"
			"ladCancl  p sum 9   Events cancelled from ladder"
			"botCancl  p sum 9   Events cancelled from bottom"
			"topScan   p sum 16  Events scanned in top"
			"ladScan   p sum 11  Events scanned from ladder"
			"botScan   p sum 11  Events scanned from bottom"
			"topIns    p sum 16  Inserts into top"
			"ladIns    p sum 14  Inserts into rungs"
			"botIns    p sum 13  Inserts into bottom"
			"rungs     p max 18  Max rung count"
			"bktRung   p sum 12  Average #buckets per rung"
			"botSize   p sum 17  Average bottom size"
			"bktSize   p sum 16  Average bucket width"	
		)
	elif [[ $SIM_PARAMS == *"heap2tQ"* ]]; then
		echo "** Gathering heap2tQ runtime stats **"		
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
	else
		echo "** Gathering generic runtime stats **"
		RunTimeStatsInfo=(
			"runtime     1 max 2   ^real "
			"memory      p max 6   Maximum resident set size"
			"rollbacks   p sum 9   Total #rollbacks "
			"mpiMsgs     p sum 7   Total #MPI messages "
			"schedules   p sum 10  Total schedules "
			"commEvts    p sum 4   Total Committed Events"
		)
	fi
}

# Convenience method to use override values specified in PARAMS as the
# only values for CmdLineParams that this script is going to explore.
# The set of overrides to be processed by this script is passed-in as
# the parameters.
#    $* : The set of parameters specified by the user.
function overrideCmdLineParams {
	# echo "params to check for override: $#"
	local paramList=""
	while [[ $# -gt 0 ]]
	do
		# Get parameter to process
		local param="$1"
		# Check if the specified parameter is in the CmdLineParams
		# array and if so, override its list of values with the one
		# specified by the user.
		if [ "${CmdLineParams[$param]+isset}" ]; then
			# Override this parameter with specified value.
			CmdLineParams[$param]="$2"
		else
			# Save this parameter and value for further use
			paramList="$paramList $param $2"
		fi 
		# Skip over parameter and its value.
		shift 2
	done
	# Setup the global parameters for further use
	SIM_PARAMS="$paramList"
}

# Helper function to gather data for parallel simulation with a given
# number of procs
#   $1: The number of parallel processes to be used.  Each compute
#       node is assigned 4 of these processes.
function generateParStats() {
	# Setup the global PROCS variable with the number of procs to use.
	PROCS="$1"
	# Setup the PBS script to be run 10 times
	local pbsScript="$TMPDIR/pbs_job.sh"
	createPBSscript "$pbsScript"
	# Run the PBS script 10 times and collect data from 10 runs
	runPBSjobs $REPS "$pbsScript"
	if [ $? -ne 0 ]; then
		# Error submitting PBS job or waiting on them.
		return 1
	fi
	# Summarize data from the REPS number of output files
	recordAllStats $OUT_FILE
	# Save the raw data in a single file in case we ever need it.
	saveOutputFiles "$pbsScript"
}

# The main function that coordinates the various activities of
# collecting data for parallel simulation runs
function parMain() {
	# Change to working directory if specified
	if [ ! -z "$PBS_O_WORKDIR" ]; then
		cd "$PBS_O_WORKDIR"
	fi
	# First process all command-line arguments and setup global
	# variables using function in included script
	parseArgs $*
	if [ $? -ne 0 ]; then
		# Invalid parameters
		exit 1
	fi 
    # Next check to ensure we have all the necessary variables setup
    checkParams
    if [ $? -ne 0 ]; then
        # Some varibles were not set (or are not valid)
        exit 1
    fi
	# Next extract override values for CmdLineParams from PARAMS
	overrideCmdLineParams $SIM_PARAMS
	echo "Using temproary directory: $TMPDIR"
	mkdir -p "$TMPDIR"
	# Setup the statistics to be gathered from each set of runs
	setupRuntimeStats
	# Save procs in CSV form and initialize PROCS for parallel sim
	local csvPROCS="$PROCS"
	PROCS=2
    # Check add header to output file (if it does not have one already)
    printHeader "$OUT_FILE"
    # Print parameters and ranges for cross verifications
    printParamRanges
    # Now generate data various combination of parmaeters.
	initCmdLineParams
	# Now generate data for each config in procs after replacing ","
	# with spaces to make string processing easier.
	procList=${csvPROCS//,/\ }
	for np in $procList
	do
		# Use helper method to collect stats for given configuration
		generateParStats $np
		if [ $? -ne 0 ]; then
			# Some error occurred
			exit 2
		fi 
	done
}

# Let the main function do all the necessary work
parMain "$*"

# End of script
