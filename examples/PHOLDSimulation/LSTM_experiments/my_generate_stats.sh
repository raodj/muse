#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# processes sim_out_*.txt files in a series of subdirectories and
# extracts 1 line of statistics for each directory.

# Run as (with parameters to simulation at the end):
#
# $ ../scripts/generate_stats.sh 

# The parameter combination and information is obtained from shared
# script
if [ -z ${sourceDir+x} ]; then
    sourceDir="$(dirname "$0")"
fi
source "${sourceDir}/params_funcs.sh"
source "${sourceDir}/my_stats_funcs.sh"

OUT_FILE="stats.csv"
PROCS=1

# ------ Typically you should not have to modify values below this line. -----

# Setup the list of values to be summarized from the data files.  The
# array contains the list of parameters whose information is to be
# summarized in the following format:
#   "param_name [1|p] [sum|max|any] col_with_stat search words"
#
# NOTE: Stats that are thread-only are tagged with a leading *
RunTimeStatsInfo=(
	"runtime      1 max 1   seconds time elapsed"
        "ghz          1 max 4   GHz"
	"memory       1 max 6   Maximum resident set size"
	"rollbacks    p sum 4   Total #rollbacks"
	"mpiMsgs      p sum 5   Total #MPI messages"
	"schedules    p sum 4   Total schedules"
	"commEvts     p sum 5   Total Committed Events"
        "defRecyHit   p sum 5   Default Recycler %hits"
        "timeWindow   p sum 9   Adaptive time window"
        "chkMpiMsg    p sum 5   #process MPI msgs calls"
        "succMpiChk   p sum 8   MPI msg batch size"
        "mpiBatchSz   p sum 10  MPI msg batch size"
        "msgChkThres  p sum 10  Max MPI msg check thres"
        "*numaRecyHit p sum 5   NUMA Recycler %hits"
        "*%dealocCall p sum 7   #Deallocs cleared/call"
        "*chkShrQChk  p sum 5   #processing of sharedQ"
        "*succShrQChk p sum 7   Avg sharedQ size"
        "*shrQBatchSz p sum 9   Avg sharedQ size"
        "*numaRedistr p sum 2   Redistributions"
        "cacheRefs    1 max 1   cache-references "
        "cacheMisses  1 max 1   of all cache refs"
        "instrs       1 max 1   insn per cycle"
        "insPerCycle  1 max 4   insn per cycle"
        "cpusUtil     1 max 5   CPUs utilized"
        # The following is a specially computed value
        "cachMisInst  1 one 2   CacheMiss/Instr"
        # The following is a specially computed value to account for
        # turbo boost on some of the cores on Ownes
        "normRuntime  1 max 2   NormalizedRuntime"
)

# Convenience method (called from extractAllStats) to extract
# statistics for a given information from a given set of output files
# in the temporary directory.  The parameters to this function are in
# the order:
#   $1: The directory containing output files.
#   $2: Parameter name (used for column titles etc.)
#   $3: Expected number of values per data file
#   $4: Type of per-file acumulation (sum, max, or any)
#   $5: The column in each reference line with statistic
#   $*: The search/grep strings to be used
function extractStats {
    # Make aliases for parameters to keep script readable
    local outDir="$1"
    local paramName="$2"
    local expNum="$3"
    local paramType="$4"
    local column=$5
    shift 5
    local searchWords=( $* )
    local searchStr="${searchWords[@]}"
    # Use helper method in stats_funcs.sh to summarize the data
    local outFile="${outDir}/sim_out_"
    local tmpFile="$TMPDIR/tmp.txt"
    local expCount=$PROCS
    if [ "$expNum" == "1" ]; then
	expCount=1
    fi
    if [ "$searchStr" == "h:mm:ss or m:ss" ]; then
	local stats=( `summarize_elapsed_time_stats "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )
    elif [ "$searchStr" == "CacheMiss/Instr" ]; then
        local stats=( `summarize_cache_miss_per_instr "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )
    elif [ "$searchStr" == "NormalizedRuntime" ]; then
        local stats=( `summarize_norm_runtime "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )
    else
	local stats=( `summarize_runtime_stats "$outFile" $expCount "$paramName" "$tmpFile" "$searchStr" $column "$paramType"` )
    fi
    # Print the necessary information as a CSV
    echo "${stats[2]}, ${stats[3]}, ${stats[4]}, ${stats[5]}, ${stats[6]}, ${stats[7]}, ${stats[8]}, ${stats[9]}, ${stats[10]}"
}

# Extract summary statistics based on the application supplied
# RunTimeStatsInfo array.
function recordAllStats {
    local outdir="$2"
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
        local paramInfo=( ${RunTimeStatsInfo[$idx]} )
        local thrOnly="${paramInfo[0]:0:1}"
        if [ "${thrOnly}" == "*" ]; then
            # Remove leading *
            paramInfo[0]="${paramInfo[0]:1}"
        fi
        if [ "${thrOnly}" == "*" -a $useMPI -eq 1 ]; then
            # This run is collecting data for MPI processes that do
            # not have thread-specific statistics. Simply set dummy
            # values.
            local currStatInfo="na, na, na, na, na, na, na, na, na"
        else
	    # Use helper method to extract this stat
	    local currStatInfo=`extractStats ${outdir} ${paramInfo[@]}`
        fi
	statInfo="$statInfo, $currStatInfo"
	# If the stat was sum we automatically generate a max value
	# for it for parallel runs -- this err'ing on the side of more
	# data than to regret about not having it in hindsight
	# local paramInfo=( ${RunTimeStatsInfo[$idx]} )
	# if [ $PROCS -gt 1 -a "${paramInfo[2]}" == "sum" ]; then
	#    # Use helper method to extract max version of this stat
	#    paramInfo[2]="max"
	#    local currStatInfo=`extractStats ${outdir} ${paramInfo[@]}`
	#    statInfo="$statInfo, $currStatInfo"
	# fi
    done
    # Finally write the data to the log file
    if [ $# -lt 3 ]; then
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
        if [ "${statInfo[0]:0:1}" == "*" ]; then
            # Remove leading *
            statInfo[0]="${statInfo[0]:1}"
        fi
	local name=${statInfo[0]}
	header="$header, ${name}_mean, ${name}_sd, ${name}_ci, ${name}_ci, ${name}_min, ${name}_box_q1, ${name}_median, ${name}_box_q2, ${name}_max"
	# if [ $PROCS -gt 1 -a "${statInfo[2]}" == "sum" ]; then
	#    # Automatically included max version of header
	#    name="${name}Max"
	#    header="$header, ${name}_mean, ${name}_sd, ${name}_ci, ${name}_ci, ${name}_min, ${name}_box_q1, ${name}_median, ${name}_box_q2, ${name}_max"
	# fi
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

# The main function that coordinates tasks in this script.
function main() {
    # First process all command-line arguments and setup global variables
    parseParamsArgs $*
    if [ $? -eq 1 ]; then
        # A specific directory is specified. Process just that.
        echo "Processing just directory $1"
        printHeader
        recordAllStats "$OUT_FILE" "$1" "print"
        exit 0
    fi 
    # Check add header to output file (if it does not have one already)
    printHeader $OUT_FILE
    # Print parameters and ranges for cross verifications
    printParamRanges
    # Run a counter to generate unique output file names
    local count=0
    # Now generate data various combination of parmaeters.
    initCmdLineParams
    while [[ 1 ]]
    do
        # Setup the number of procs from threads/procs in the combos
        if [ $useMPI -eq 0 ]; then
            PROCS=${currCmdLineVals['--threads-per-node']}
        else
			PROCS=${currCmdLineVals['procs']}
        fi
        # Get the output directory that contains files for this
        # combination of parameter values using method in
        # params_funcs.sh script.
        local outdir=`toPathName`
	# Summarize data from the REPS number of output files
        local outFile="tmp_out_${count}.txt"
	recordAllStats "${outFile}" "${outdir}" &
	# Generate next set of parameters
	nextParamComb
        local combosDone=$?
	if [ $combosDone -ne 0 -o $count -eq 5 ]; then
	    # Done exploring different parameter combinations.
            echo "Waiting for $count processes..."
            wait
            # Now aggregate all the output
            for i in `seq 0 ${count}`
            do
                local outFile="tmp_out_${i}.txt"
                cat "${outFile}" >> "${OUT_FILE}"
                rm -f "${outFile}"
            done
            if [ $combosDone -ne 0 ]; then 
	        return 0
            fi
            count=0
        else
            count=$(( count + 1 ))
	fi
    done
}

# Call the main function to process the arguments
main $*

# End of script
