#!/bin/bash

#PBS -S /bin/bash
#PBS -j eo

# This is a shell script that can either be run standalone or via PBS
# as a PBS script.  This shell script performs the following tasks:
#
#  1. Uses a given set of parameters and range of values.
#  2. Generates a set of Sobol random numbers
#  3. Translates Sobol numbers to parameter range
#  4. Runs PHOLD simulations using 2 different scheduler queues
#  5. Appends values to results files.

# The simulation executable to be used.  Typically it is a symbolic
# link to the actual executable.
SIM_EXEC="./phold"

# The default values for the two types of queues we are working with
QUEUE1_DEFAULT="ladderQ"
QUEUE2_DEFAULT="heap2tQ"

# The list of command-line parameters to be explored for GSA.  These
# values change from simulation-to-simulation.  Entries are in the
# form: cmd-line-arg="min max".  If cmd-line-arg name ends with a '%'
# sign (e.g.: --imbalance%), then the range is converted to a
# fraction.

declare -A CmdLineParams
CmdLineParams=( ['--rows']="10 100"
                ['--cols']="10 100"
                ['--delay']="1 10"
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
ADDL_SIM_CMD_LINE_ARGS="--2t-ladderQ-t2k 1"

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

# Helper function to run 2 sequential simulations with 2 different
# queues and record the elapsed time.
function compare_queues() {
    local cmdLine="$1"
    local pid="$$"
    local suffix="${pid}_timing.txt"
    echo "Running simulations for: $cmdLine..."
    # Repeat simulations for 3 times and average the values.
    for i in `seq 0 2`
    do
        # Set output files to record timings without mixing outputs
        local queue1_log="${TMPDIR}/${QUEUE1}_${i}_${suffix}"
        local queue2_log="${TMPDIR}/${QUEUE2}_${i}_${suffix}"
        # Run the two simulation in parallel (assume 2 cores are allocated)
        /usr/bin/time -p -o "${queue1_log}" $SIM_EXEC $cmdLine --scheduler-queue $QUEUE1 ${ADDL_SIM_CMD_LINE_ARGS} > /dev/null &
        /usr/bin/time -p -o "${queue2_log}" $SIM_EXEC $cmdLine --scheduler-queue $QUEUE2 ${ADDL_SIM_CMD_LINE_ARGS} > /dev/null &
    done
    # Wait for all 6 processes to finish
    wait
    # Extract timings for the 6 runs
    local q1_time=( 0 0 0 )
    local q2_time=( 0 0 0 )
    for i in `seq 0 2`
    do
        # Set output files to record timings without mixing outputs
        local queue1_log="${TMPDIR}/${QUEUE1}_${i}_${suffix}"
        local queue2_log="${TMPDIR}/${QUEUE2}_${i}_${suffix}"
        # Extract timings for the 2 queues
        q1_time[$i]=`head -1 "${queue1_log}" | cut -d" " -f2`
        q2_time[$i]=`head -1 "${queue2_log}" | cut -d" " -f2`
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
    echo "TMPDIR=$TMPDIR"
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

