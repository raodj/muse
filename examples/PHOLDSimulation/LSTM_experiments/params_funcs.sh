#!/bin/bash

# This is a convenience script to generate parameter combinations used
# to streamline running experiments.  The methods in this file are
# used by different scripts in this directory.

# The list of command-line parameters to be varied for simulation.
# These values change from simulation-to-simulation.  Entries are in
# the form: cmd-line-arg="values".  Arguments with a '%' sign at
# the end are converted to percentage values in the range 0.0 to 1.0

# The following configuration was used for sequential simulations for
# recording caching charachteristics using perf.
declare -A CmdLineParams

# Fixed subset of command-line arguments to be passed to PBS
PBS_PARAMS=""

# Any additional command-line options to be included.
ADDL_SIM_CMD_LINE_ARGS="--adapt-time-window"

# ------ Typically you should not have to modify values below this line. -----

# Flag to indicate if MPI or threads are to be used
useMPI=1
if [ ${CmdLineParams['--threads-per-node']+abs} ]; then
    useMPI=0
    SIM_EXEC_DEF_PARAMS="${SIM_EXEC_DEF_PARAMS} --simulator mpi-mt"
fi

# Flag to indicate if multi-mt-queues value should be auto set to 1/2
# the number of threads.
autoSubQueues=0

# The default number of REPS to run
REPS=10

# The global array of current values for the parameters to be
# populated by the initCmdLineParams method in this script.
declare -A currCmdLineVals

# The current index of parameter in CmdLineParams that is being used
# This value is used to generate various combinations
declare -A currCmdLineIndexs

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
function parseParamsArgs() {
    while [[ $# > 0 ]]
    do
        key="$1"
        shift
        case $key in
            --params)
                SIM_PARAMS="$*"
                return 0
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
            --auto-mt-sub-queues)
                autoSubQueues=1
                ;;
            *)
                echo "Unknown option $key encountered"
                return 1
                ;;
        esac
    done
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
        if [ ${param:0:1} == '-' ]; then
            local paramVal=`param2CmdLine "$param"`
            cmdLine="${cmdLine} $paramVal"
        fi
    done
    # If auto mt-subqueues is set then add one more parameter
    if [ $useMPI -eq 0 -a $autoSubQueues -eq 1 ]; then
        local queues=$(( ${currCmdLineVals['--threads-per-node']} / 2 ))
        if [ $queues -lt 1 ]; then
            queues=1
        fi
        cmdLine="${cmdLine} --multi-mt-queues ${queues}"
    fi
    # Echo the command-line back to the caller
    echo "$cmdLine"
}

# Convenience function to convert the current set of parameter values
# to a suitable file/directory name.
#
function toPathName() {
    # The file/directory name being built by this function
    local pathName=""
    # Convert each parameter value to command-line argument
    for param in "${!CmdLineParams[@]}"
    do
        # Get parameter value
        local val=${currCmdLineVals[$param]}
        # Extract the first word of the parameter for path
        # For example, extract 'threads' from '--threads-per-node'
        local words=( ${param//-/\ } )
        local word=${words[0]}
        # Extract the value and 1st word of the command-line arg
        pathName="${pathName}_${word}_${val}"
    done
    # Echo the path name back to the caller ignoring the leading '_'
    echo "${pathName:1}"
}

# End of script
