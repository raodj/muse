#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# submits multiple PBS jobs to run simulations in parallel.  Run as:
#
# $ ./gen_parallel_gsa_data.sh --reps 100 --skip 10 --outfile par_gsa.csv > par_gsa_log.txt 2>&1 &

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

# The default values for the two types of queues we are working with
QUEUE1_DEFAULT="2tLadderQ"
# QUEUE2_DEFAULT="heap2tQ"
QUEUE2_DEFAULT="ladderQ"

# The list of command-line parameters to be explored for GSA.  These
# values change from simulation-to-simulation.  Entries are in the
# form: cmd-line-arg="min max"

declare -A CmdLineParams
CmdLineParams=( ['--rows']="10 100"
                ['--cols']="10 100"
                ['--delay']="0 12"
                ['--selfEvents%']="0 100"
                ['--eventsPerAgent']="1 20"
                ['--granularity']="0 50"
                ['--simEndTime']="300 600"
                ['--imbalance%']="0 100"
                ['--gvt-delay']="100 10000"
              )

# ------ Typically you should not have to modify values below this line. -----

# The Sobol random number generation program.
SOBOL_GEN="./Sobol"

# Any additional command-line options to be included.
ADDL_SIM_CMD_LINE_ARGS="--2t-ladderQ-t2k 32 --time-window 20"

# Fixed subset of command-line arguments to be passed to PBS
PBS_PARAMS="-j oe -lnodes=1:ppn=4 -lwalltime=00:05:00 -lmem=4gb -m n"

# The temporary directory to be used by this script
TMPDIR="$$_par_tmp"

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
            --reps)
                NUM_REPS=$1
                shift
                ;;
            --skip)
                SKIP_VALS=$1
                shift
                ;;
            --outfile)
                OUT_FILE="$1"
                shift
                ;;
            --queue1)
                QUEUE1="$1"
                shift
                ;;
            --queue2)
                QUEUE2="$1"
                shift
                ;;
            --tmpdir)
                TMPDIR="$1"
                shift
                ;;
            *)
                echo "Unknown option $1 encountered"
                showHelpAndExit
                ;;
        esac
    done
}

function checkParams() {
    if [ -z "$NUM_REPS" ]; then
        echo "Value of NUM_REPS (--reps) not set."
        return 1
    fi
    if [ -z "$SKIP_VALS" ]; then
        echo "Value for SKIP_VALS (--skip) not set."
        return 1
    fi
    if [ -z "$OUT_FILE" ]; then
        echo "Output log file OUT_FILE (--outfile) not set."
        return 1
    fi
    if [ -z "$QUEUE1" ]; then
        QUEUE1=${QUEUE1_DEFAULT}
    fi
    if [ -z "$QUEUE2" ]; then
        QUEUE2=${QUEUE2_DEFAULT}
    fi
    if [ -z "$TMPDIR" ]; then
        TMPDIR="/tmp"
    fi
    # All variables look good
    return 0
}

# Convenience function to convert a Sobol random number to value
# associated with a single parameter.
#   $1: Parameter key
#   $2: Sobol random number
function param2CmdLine() {
    # Create aliases for parameter to streamline script.
    local key="$1"
    local rndNum="$2"
    # Get min & max values for the specified parameter
    local paramRange=( ${CmdLineParams[$param]} )
    # Extract min and max to make formula below easier to read
    local min=${paramRange[0]}
    local max=${paramRange[1]}
    local range=$(( max - min ))
    # Scale the rndNum to min and max values
    local paramVal=`echo "$min + $rndNum * $range" | bc -l | cut -d'.' -f1`
	# The above cut command gives empty string for values < 1
	if [ -z "$paramVal" ]; then
		paramVal="0"
	fi	
    # If the parameter is a percentage convert it to 0 to 1.0 value
    if [ "${key: -1}" == "%" ]; then
        paramVal=`echo "$paramVal / 100.0" | bc -l`
		# Round to 3 decimal places
		paramVal=`printf "%.3f" ${paramVal}`		
        # Remove traling % from key.
        # key="${key:0:-1}"
		local len=$(( ${#key} - 1 ))
		key=${key:0:len}
    fi
    # Echo the parameter and its value
    echo "$key $paramVal"
}

# Convenience function to convert Sobol random numbers to
# corresponding command-line argument values.
#    $* : A set of Sobol random numbers (each in range 0 to 1.0)
#
function toCmdLine() {
    # The set of sobol random numbers to work with.
    local rndNums=( $* )
    # The command-line being built by this function
    cmdLine=""
    # Convert each Sobol random number to corresponding parameter value.
    local idx=0
    for param in "${!CmdLineParams[@]}"
    do
        local paramVal=`param2CmdLine "$param" ${rndNums[$idx]}`
        cmdLine="${cmdLine} $paramVal"
        # Skip to next Sobol random number
        idx=$(( idx + 1 ))
    done
    # Echo the command-line back to the caller
    echo "$cmdLine"
}

# Helper method to add a simple header to the output CSV file to make
# the data easy to interpret
function checkAddHeader() {
    if [ ! -f "$OUT_FILE" ]; then
        # Print fixed timestamp column first
        echo -n "# 'Timestamp', " >> "$OUT_FILE"
        # Print the command-line parameters for verifications
        for param in "${!CmdLineParams[@]}"
        do
            paramRange=( ${CmdLineParams[$param]} )
            echo -n "'$param ${paramRange[@]}', " >> "$OUT_FILE"
        done    
        # Add fixed timings for the 2 queues
        echo "${QUEUE1}_timing_secs, ${QUEUE2}_timing_secs" >> "$OUT_FILE"
    fi
}

# Convenience function to log timings and parameter values from
# command-line arguments.
#  $1: Timing for queue1
#  $2: Timing for queue2
#  $3: Command-line arguments supplied to the simulation.
function logGSAdata() {
    local time1=$1
    local time2=$2
    local params=( $3 )
    local logLine=`date`
    # First print a comma-separated list of parameter values only
    # (don't print name of command-line argument)
    for i in `seq 1 2 ${#params[@]}`
    do
        logLine="${logLine}, ${params[$i]}"
    done
    # Echo log entry with timings to output log file.
    echo "${logLine}, ${time1}, ${time2}" >> "$OUT_FILE"
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

# Convenience method to extract runtime of a simulation from the time
# logs.  If the time logs is not available and the output logs show
# that the job exceeded wall time, then this method returns the
# maximum runtime of 300 seconds as default value.
#   $1: Log file with output from time command
#   $2: Log file with output/error stream from the process.
#
function getRunTime() {
    # Set output files with timings
    local queue_log="$1"
	local out_log="$2"
    # Extract timings for the 2 queues
    q_time=`head -1 "${queue_log}" | cut -d" " -f2`
	# Fix time to max if data is not available -- this assumes
	# walltime exceeded by default.
	if [ -z "${q_time}" ]; then
		# Possibly the job exceeded wall time. Check to ensure this is
		# indeed the case by searching for line with the format:
		# >> PBS: job killed: walltime 318 exceeded limit 300
		local errLine=( `grep "job killed: walltime" ${out_log} | grep "exceeded limit"` )
		if [ ${#errLine[@]} -gt 2 ]; then
			# Yes indeed wall time was exceeded. Return maximum time
			# limit which is the last word in the errLine as the runtime
			local lastIdx=$(( ${#errLine[@]} - 1 ))
			q_time=${errLine[$lastIdx]}
		fi
	fi
	# Echo the queue time for caller to use
	echo $q_time
	return 0
}

# Helper function to run 2 sequential simulations with 2 different
# queues and record the elapsed time.
function compare_queues() {
    local cmdLine="$1"
    local pid="$$"
    local suffix="${pid}_timing.txt"
	local q1PBS="${TMPDIR}/${pid}_q1_pbs.sh"
	local q2PBS="${TMPDIR}/${pid}_q2_pbs.sh"
    echo "Running simulations for: $cmdLine..."
	# Create a couple of PBS scripts with the necessary parameters:
	echo "cd \$PBS_O_WORKDIR" > "$q1PBS"
	echo "/usr/bin/time -p -o \$TIME_LOG mpiexec $SIM_EXEC $cmdLine --scheduler-queue $QUEUE1 ${ADDL_SIM_CMD_LINE_ARGS}" >> "$q1PBS"
	echo "cd \$PBS_O_WORKDIR" > "$q2PBS"	
	echo "/usr/bin/time -p -o \$TIME_LOG mpiexec $SIM_EXEC $cmdLine --scheduler-queue $QUEUE2 ${ADDL_SIM_CMD_LINE_ARGS}" >> "$q2PBS"
    # Submit 3 PBS jobs per queue and save job IDs of PBS jobs
	local q1Jobs=( x x x )
	local q2Jobs=( x x x )
    for i in `seq 0 2`
    do
        # Set output files to record timings without mixing outputs
        local queue1_log="${TMPDIR}/${QUEUE1}_${i}_${suffix}"
        local queue2_log="${TMPDIR}/${QUEUE2}_${i}_${suffix}"
		# Set name for the jobs
		local name1="q1_${i}"
		local name2="q2_${i}"
		# Setup names of the output files for the job(s)
		local q1_out="${TMPDIR}/q1_${i}_out.txt"
		local q2_out="${TMPDIR}/q2_${i}_out.txt"
		# Submit PBS jobs and record their job IDs
		q1Jobs[$i]=`qsub ${PBS_PARAMS} -N $name1 -v "TIME_LOG=${queue1_log}" -o $q1_out $q1PBS`
		q2Jobs[$i]=`qsub ${PBS_PARAMS} -N $name2 -v "TIME_LOG=${queue2_log}" -o $q2_out $q2PBS`
    done
    # Wait for all 6 processes to finish
    waitForPBS ${q1Jobs[@]} ${q2Jobs[@]}
    # Extract average timings for the two different queues
    # Extract timings for the 6 runs
    local q1_time=( 0 0 0 )
    local q2_time=( 0 0 0 )
    for i in `seq 0 2`
    do
        # Set output files to record timings without mixing outputs
        local queue1_log="${TMPDIR}/${QUEUE1}_${i}_${suffix}"
        local queue2_log="${TMPDIR}/${QUEUE2}_${i}_${suffix}"
		# Setup names of the output files for the job(s)
		local q1_out="${TMPDIR}/q1_${i}_out.txt"
		local q2_out="${TMPDIR}/q2_${i}_out.txt"		
        # Extract timings for the 2 queues
        q1_time[$i]=`getRunTime "$queue1_log" "$q1_out"`
        q2_time[$i]=`getRunTime "$queue2_log" "$q2_out"`
    done
    # Compute average time from the 3 runs
    local queue1_time=`echo "( ${q1_time[0]} + ${q1_time[1]} + ${q1_time[2]} ) / 3" | bc -l`
    local queue2_time=`echo "( ${q2_time[0]} + ${q2_time[1]} + ${q2_time[2]} ) / 3" | bc -l`
    # Round timings to 3 decimals for convenience
    queue1_time=`printf "%.3f" ${queue1_time}`
    queue2_time=`printf "%.3f" ${queue2_time}`
    # Print timings for checking
    echo "    ${QUEUE1} timings: $queue1_time ( ${q1_time[@]} )"
    echo "    ${QUEUE2} timings: $queue2_time ( ${q2_time[@]} )"
    # Log the parameters and the timings observed
    logGSAdata "${queue1_time}" "${queue2_time}" "${cmdLine}"
}

# The main function that coordinates tasks in this script.
function main() {
	# Change to working directory if specified
	if [ ! -z "$PBS_O_WORKDIR" ]; then
		cd "$PBS_O_WORKDIR"
	fi
    # First process all command-line arguments and setup global variables
    parseArgs $*
    # Next check to ensure we have all the necessary variables setup
    checkParams
    if [ $? -ne 0 ]; then
        # Some varibles were not set (or are not valid)
        exit 1
    fi
	echo "Using temproary directory: $TMPDIR"
	mkdir -p "$TMPDIR"
    # Check add header to output file (if it does not have one already)
    checkAddHeader
    # Print parameters and ranges for cross verifications
    printParamRanges
    # Now generate NUM_REPS Sobol numbers and simulate each configuration.
    local paramCount=${#CmdLineParams[@]}
    ${SOBOL_GEN} ${paramCount} ${NUM_REPS} ${SKIP_VALS} | \
        while IFS='' read -r rndNums || [[ -n "$rndNums" ]]; do
            # echo "Sobol random numbers: $rndNums"
            local cmdLine=`toCmdLine ${rndNums}`
            # echo "cmdLine = $cmdLine"
            # Record the time for 2 queues using helper function
            compare_queues "$cmdLine"
        done
}

# Run the main function
main $*

# End of script
