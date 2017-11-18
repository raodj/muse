#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run simulations in parallel.  Run as
# (with parameters to simulation at the end):
#
# $ ./generate_stats.sh --reps 10 --procs 4 --outfile ph4_ladderQ_4proc_timings.csv --params '--rows 100 --cols 100 --scheduler-queue ladderQ --simEndTime 500' > ph4_ladderQ_4proc_log.txt 2>&1 &

# This shell script performs the following tasks:
#
#  1. Uses a given set of parameters and range of values.
#  2. Generates a set of Sobol random numbers
#  3. Translates Sobol numbers to parameter range
#  4. Submits 3 PHOLD parallel simulations for each of the 2 different
#     scheduler queues
#  5. It waits for the jobs to complete.
#  6. It computes average time from 3 runs and logs it.

# The simulation executable to be used.  Typically it is a symbolic
# link to the actual executable.
SIM_EXEC="./phold"

# An optional perf command-line that can be overridden to use perf to
# collect additional statistics
PERF_EXEC=""

# The list of command-line parameters to be varied for simulation.
# These values change from simulation-to-simulation.  Entries are in
# the form: cmd-line-arg="values".  Arguments with a '%' sign at
# the end are converted to percentage values in the range 0.0 to 1.0

# The following configuration was used for sequential simulations for
# recording caching charachteristics using perf.
declare -A CmdLineParams
CmdLineParams=( ['--delay']="1 10"
                ['--selfEvents%']="25"
                ['--eventsPerAgent']="1 2 5 10 15 20"
              )

# The following configuration was used for sequential simulations
# declare -A CmdLineParams
# CmdLineParams=( ['--delay']="1 2 5 10"
#                ['--selfEvents%']="0 25"
#                ['--eventsPerAgent']="1 2 5 10 15 20"
#              )

# The following configuration is used for parallel simulations
# declare -A CmdLineParams
# CmdLineParams=( ['--delay']="1 10"
#                ['--selfEvents%']="0 25"
#                ['--eventsPerAgent']="2 5 10"
#              )

# declare -A CmdLineParams
#CmdLineParams=( ['--2t-ladderQ-t2k']="1 2 16 32 64 128 256 512 1024 2048" )

# The global array of current values for the parameters to be
# populated by the initCmdLineParams method in this script.
declare -A currCmdLineVals

# The current index of parameter in CmdLineParams that is being used
# This value is used to generate various combinations
declare -A currCmdLineIndexs

# ------ Typically you should not have to modify values below this line. -----

# Fixed subset of command-line arguments to be passed to PBS
PBS_PARAMS="-lwalltime=01:00:00"

# Any additional command-line options to be included.
ADDL_SIM_CMD_LINE_ARGS="--time-window 10"

# Include helper functions
source ./stats_funcs.sh

# Convenience function print parameters and ranges set
function printParamRanges() {
    # Print the command-line parameters for verifications
    for param in "${!CmdLineParams[@]}"
    do
        paramRange=( ${CmdLineParams[$param]} )
        echo "$param ${paramRange[@]}"
    done    
}

# Convenience function to check and set variables using command-line
# arguments.  The command line parameters to the script are passed-in
# as the parameters to this function from main
function parseArgs() {
    while [[ $# > 0 ]]
    do
        key="$1"
        shift
        case $key in
            --procs)
                PROCS=$1
                shift
                ;;
            --params)
                SIM_PARAMS="$*"
                return 0
                ;;
            --outfile)
                OUT_FILE="$1"
                shift
                ;;
            --tmpdir)
                TMPDIR="$1"
                shift
                ;;
			--reps)
				REPS="$1"
				shift
				;;
			--simExec)
				SIM_EXEC="$1"
				shift
				;;
            *)
                echo "Unknown option $key encountered"
                return 1
                ;;
        esac
    done
	return 0
}

# Convenience function to ensure that necessary parameter values have
# been specified.
function checkParams() {
    if [ -z "$PROCS" ]; then
        echo "Value of PROCS (--procs) not set."
        return 1
    fi
    if [ -z "$SIM_PARAMS" ]; then
        echo "Value for SIM_PARAMS (--params) not set."
        return 1
    fi
    if [ -z "$OUT_FILE" ]; then
        echo "Output log file OUT_FILE (--outfile) not set."
        return 1
    fi
    if [ -z "$TMPDIR" ]; then
        TMPDIR="./tmp_$$"
    fi
	if [ -z "$REPS" ]; then
		REPS=10
	fi
    # All variables look good
    return 0
}

# Helper function to initialize the values of currCmdLineVals using
# starting values for the different parameters.
function initCmdLineParams {
    for param in "${!CmdLineParams[@]}"
    do
        local paramInfo=( ${CmdLineParams[$param]} )
		currCmdLineVals[$param]=${paramInfo[0]}
		currCmdLineIndexs[$param]=0
    done	
}

# Function to change currCmdLineVals to the next combination updating
# each value as needed.
function nextParamComb() {
    for param in "${!CmdLineParams[@]}"
    do
		local nextIdx=$(( ${currCmdLineIndexs[$param]} + 1 ))
        local valList=( ${CmdLineParams[$param]} )
		if [ $nextIdx -eq ${#valList[@]} ]; then
			# All values used. Reset back to 0
			nextIdx=0
		fi 
		# Update parameter value with updated value
		currCmdLineVals[$param]=${valList[$nextIdx]}
		currCmdLineIndexs[$param]=$nextIdx
		# If a wrap around has not occurred, nothing much to do
		if [ $nextIdx -gt 0 ]; then
			# No further change is necessary.
			return 0
		fi
    done
	# When control drops here the last parameter value was updated. So
	# we are done with all the possible parameters values!
	return 1
}

# Convenience function to convert current parameter value to a
# suitable command-line  parameter
#    $1: Parameter key
#
function param2CmdLine() {
    # Create aliases for parameter to streamline script.
    local key="$1"
	local val=${currCmdLineVals[$key]}
    # If the parameter is a percentage convert it to 0 to 1.0 value
    if [ "${key: -1}" == "%" ]; then
        val=`echo "$val / 100.0" | bc -l`
		# Round the value to 3 decimals
		val=`printf "%0.3f" $val`
        # Remove traling % from key.
        # key="${key:0:-1}"
		local len=$(( ${#key} - 1 ))
		key=${key:0:len}
    fi
    # Echo the parameter and its value
    echo "$key $val"
}

# Convenience function to convert the current set of parameter values
# to corresponding command-line argument values.
#
function toCmdLine() {
    # The command-line being built by this function
    cmdLine="${SIM_PARAMS}"
    # Convert each parameter value to command-line argument
    for param in "${!CmdLineParams[@]}"
    do
        local paramVal=`param2CmdLine "$param"`
        cmdLine="${cmdLine} $paramVal"
    done
    # Echo the command-line back to the caller
    echo "$cmdLine"
}

# The list of jobs on which to wait are specified as the arguments to
# this function.
#    $* : List of PBS job ids to wait on.
function waitForPBS() {
	local idList="$*"
	echo "Waiting for following jobs to finish: ${idList}"
	echo "    Waiting for: $1"
	while [[ $# > 0 ]]
	do
		local state=( `qstat -f $1 | grep "job_state"` )
		if [ ${#state[@]} -ne 3 ]; then
			# Invalid job or it is already out of the queue.
			shift
			echo "    Waiting for: $1"
		elif [ ${state[2]} == 'C' ]; then
			shift
			echo "    Waiting for: $1"			
		else
			# Check on job status after 5 seconds
			sleep 5
		fi
	done
}

# Helper function to create the PBS script to be used to submit PBS
# jobs.  The path to the PBS script is passed-in as parameter.
#    $1: The path to PBS script to be generated.
#
function createPBSscript {
	local script="$1"
	local args=`toCmdLine`
	echo "Creating script for: $args"
	local mainCmd="time -p mpiexec --output-filename \$out_file /usr/bin/time -p -v $PERF_EXEC $SIM_EXEC $ADDL_SIM_CMD_LINE_ARGS $args"
	echo "#PBS -j oe"                     > "$script"
	echo "cd \$PBS_O_WORKDIR"            >> "$script"
	echo 'jobID=`echo ${PBS_JOBID} | cut -d"." -f1`' >> "$script"
	echo "out_file=\"$TMPDIR/sim_out_\${jobID}_\""   >> "$script"
	echo "fullOut=\"$TMPDIR/sim_out_\${REP}.txt\""   >> "$script"
	echo "$mainCmd"                                  >> "$script"
	echo 'cat "$out_file"* > "$fullOut"'             >> "$script"
	echo 'rm -f "$out_file"*'                        >> "$script"
}

# Function to run a given script 10 times and collate the data
#    $1: The number of reps
#    $2: The PBS script to be run
#
function runPBSjobs {
	local reps=$1
	local script="$2"
	# Compute nodes and ppn from parameter
	local nodes=$(( PROCS / 4 ))
	local ppn=$PROCS
	if [ $nodes -lt 1 ]; then
		nodes=1
	fi
	if [ $ppn -gt 4 ]; then
		ppn=4
	fi
	# Setup parameters to be passed to PBS
	local params="${PBS_PARAMS} -lnodes=$nodes:ppn=$ppn"
	# Submit reps number of PBS jobs
	local jobIDs=""
	for rep in `seq 1 $reps`
	do
		local name="sim_$$_$rep"
		local outFile="$TMPDIR/sim_pbs_out_${rep}.txt"
		echo qsub ${params} -N $name -v "REP=${rep}" $script
		jobid=`qsub ${params} -N $name -v "REP=${rep}" -o $outFile $script`
		if [ $? -ne 0 ]; then
			echo "Error submitting PBS job using following config:"
			echo "qsub ${params} -N $name -o $outFile $script"
			exit 1
		fi
		# Track jobid
		jobIDs="$jobIDs $jobid"
	done
	# Wait for jobs to finish
	waitForPBS $jobIDs
	# Things went fine so far
	return 0
}

# Convenience method (called from extractAllStats) to extract
# statistics for a given information from a given set of output files
# in the temporary directory.  The parameters to this function are in
# the order:
#   $1: Parameter name (used for column titles etc.)
#   $2: Expected number of values per data file
#   $3: Type of per-file acumulation (sum, max, or any)
#   $4: The column in each reference line with statistic
#   $*: The search/grep strings to be used
function extractStats {
	# Make aliases for parameters to keep script readable
	local paramName="$1"
	local expNum="$2"
	local paramType="$3"
	local column=$4
	shift 4
	local searchWords=( $* )
	local searchStr="${searchWords[@]}"
	# Use helper method in stats_funcs.sh to summarize the data
	local outFile="$TMPDIR/sim_out_"
	local tmpFile="$TMPDIR/tmp.txt"
	local expCount=$PROCS
	if [ "$expNum" == "1" ]; then
		expCount=1
	fi
	if [ "$searchStr" == "h:mm:ss or m:ss" ]; then
		local stats=( `summarize_elapsed_time_stats "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )		
	else
		local stats=( `summarize_runtime_stats "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )
	fi
	# Print the necessary information as a CSV
	echo "${stats[2]}, ${stats[3]}, ${stats[4]}, ${stats[5]}, ${stats[6]}, ${stats[7]}, ${stats[8]}, ${stats[9]}, ${stats[10]}"
}

# Extract summary statistics based on the application supplied
# RunTimeStatsInfo array.
function recordAllStats {
	local statInfo=`date`
	statInfo="$statInfo, $PROCS"
    # Convert each parameter value to command-line argument
    for param in "${!CmdLineParams[@]}"
    do
        local paramVal=( `param2CmdLine "$param"` )
        statInfo="$statInfo, ${paramVal[1]}"
    done
	# Now generate summary information for each statistic
	local StatCount=$(( ${#RunTimeStatsInfo[@]} - 1 ))
	for idx in `seq 0 ${StatCount}`
	do
		# Use helper method to extract this stat
		local currStatInfo=`extractStats ${RunTimeStatsInfo[$idx]}`
		statInfo="$statInfo, $currStatInfo"
		# If the stat was sum we automatically generate a max value
		# for it for parallel runs -- this err'ing on the side of more
		# data than to regret about not having it in hindsight
		local paramInfo=( ${RunTimeStatsInfo[$idx]} )
		if [ $PROCS -gt 1 -a "${paramInfo[2]}" == "sum" ]; then
			# Use helper method to extract max version of this stat
			paramInfo[2]="max"
			local currStatInfo=`extractStats ${paramInfo[@]}`
			statInfo="$statInfo, $currStatInfo"
		fi
	done
	# Finally write the data to the log file
	if [ $# -gt 0 ]; then
		# Record stats into the specified output file
		echo "$statInfo" >> "$1"
	else
		echo "$statInfo"
	fi
}

# Helper function to print column titles for the different values
# recorded in the given CSV file.
function printHeader {
	local header="# Timestamp, procs"
	# First list names of the parameters
    for param in "${!CmdLineParams[@]}"
    do
        header="$header, $param"
    done
	# Add names of stats for the different parameters listed by application
	local StatCount=$(( ${#RunTimeStatsInfo[@]} - 1 ))
	for idx in `seq 0 ${StatCount}`
	do
		local statInfo=( ${RunTimeStatsInfo[$idx]} )
		local name=${statInfo[0]}
		header="$header, ${name}_mean, ${name}_sd, ${name}_ci, ${name}_ci, ${name}_min, ${name}_box_q1, ${name}_median, ${name}_box_q2, ${name}_max"
		if [ $PROCS -gt 1 -a "${statInfo[2]}" == "sum" ]; then
			# Automatically included max version of header
			name="${name}Max"
			header="$header, ${name}_mean, ${name}_sd, ${name}_ci, ${name}_ci, ${name}_min, ${name}_box_q1, ${name}_median, ${name}_box_q2, ${name}_max"
		fi
	done
	# Finally print the header the specified data file (if any)
	if [ $# -eq 0 ]; then 
		echo "$header"
	else
		local file="$1"
		if [ ! -e "$file" ]; then
			echo "$header" > "$file"
		fi 
	fi 
}

# This is a convenience method that is used to aggregate all the
# outputs into a single file for possible use at a later date.
#   $1: The PBS script used to run the commands

function saveOutputFiles {
	local pbsScript="$1"
	# Check and create temporary-subdirectory
	local dir="$TMPDIR/aggr_out"
	mkdir -p "$dir"
	# Create file name with parameter values in the file name.
	local aggrFileName="aggr_out_proc_${PROCS}"
    for param in "${!CmdLineParams[@]}"
    do
        local paramVal=( `param2CmdLine "$param"` )
		local param=`echo ${paramVal[0]} | tr -d '\-\%'`
        aggrFileName="${aggrFileName}_${param}_${paramVal[1]}"
    done
	aggrFileName="${dir}/${aggrFileName}.txt"
	echo "aggrFileName = $aggrFileName"
	# Copy the raw data to the aggregate file
	cat "$pbsScript" > "$aggrFileName"
	# Append all the output files in order
	local outFile="$TMPDIR/sim_out"
	for rep in `seq 1 $REPS`
	do
		echo "----- run $rep -----" >> "$aggrFileName"
		local rawDataFile="${outFile}_${rep}.txt"
		cat "$rawDataFile" >> "$aggrFileName"
	done
}

# The main function that coordinates tasks in this script.
function main() {
	# Change to working directory if specified
	if [ ! -z "$PBS_O_WORKDIR" ]; then
		cd "$PBS_O_WORKDIR"
	fi
    # First process all command-line arguments and setup global variables
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
	echo "Using temproary directory: $TMPDIR"
	mkdir -p "$TMPDIR"
    # Check add header to output file (if it does not have one already)
    printHeader $OUT_FILE
    # Print parameters and ranges for cross verifications
    printParamRanges
    # Now generate data various combination of parmaeters.
	initCmdLineParams
	while [[ 1 ]]
	do
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
		# Generate next set of parameters
		nextParamComb
		if [ $? -ne 0 ]; then
			# Done exploring different parameter combinations.
			return 0
		fi
	done
}

# End of script
