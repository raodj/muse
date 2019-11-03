#!/bin/bash

# This is a script has a set of helper functions that are used to
# submit jobs.

# The simulation executable to be used.  Typically it is a symbolic
# link to the actual executable.
SIM_EXEC="./phold"

# The default set of command-line arguments to pass to the executable
#SIM_EXEC_DEF_PARAMS="--rows 100 --cols 100 --eventsPerAgent 20 --selfEvents 0 --delay 10 --simEndTime 5000 --scheduler-queue 3tHeap"

# Use this (instead of the above) for quick testing
SIM_EXEC_DEF_PARAMS="--rows 25 --cols 25 --eventsPerAgent 20 --selfEvents 0 --delay 10 --simEndTime 500"

# An optional perf command-line that can be overridden to use perf to
# collect additional statistics
PERF_EXEC="perf stat -e cpu-clock,task-clock,cycles,instructions,cache-references,cache-misses"

# Include some standard command-line options and helper functions
if [ -z ${sourceDir+x} ]; then
    sourceDir="$(dirname "$0")"
fi
source "${sourceDir}/params_funcs.sh"

# ------ Typically you should not have to modify values below this line. -----

# Convenience helper method to generate part of PBS script that checks
# for outputs from already finished runs to setup the starting rep.
#    $1: The path to PBS script to be generated.
#    $2: The subdirectory for output files.
function setupInitRep {
    local script="$1"
    local outDir="$2"
    # The following command is used to detect the output file with the
    # highest rep number.
    local hiRep='ls -1 sim_out_*.txt | tr "_" "." | cut -d"." -f3 | sort -nr | head -1'
    echo 'ST_REP=1'                                  >> "$script"
    echo 'if [ -f ${outdir}/sim_out_1.txt ]; then'   >> "$script"
    echo '  cd ${outdir}'                            >> "$script"
    echo "  ST_REP=\`${hiRep}\`"                     >> "$script"
    echo '  cd -'                                    >> "$script"
    echo 'fi'                                        >> "$script"
    echo 'echo ST_REP = ${ST_REP}'                   >> "$script"
}


function printCurrentConfig {
    echo -n "Creating script for: "
	for i in "${!currCmdLineVals[@]}"
    do
    	echo -n "$i ${currCmdLineVals[$i]} "
    done
    echo -e "\n -- using arguments: $args"
}

# Helper function to create the PBS script to be used to submit PBS
# jobs.  The path to the PBS script is passed-in as parameter.
#    $1: The path to PBS script to be generated.
#    $2: The subdirectory for output files.
#    $3: The number of processes to use.
function createMPIPBSscript {
    local script="$1"
    local outDir="$2"
    local procs="$3"
    local args=`toCmdLine`
	
	printCurrentConfig

	local mainCmd="/usr/bin/time -p -v $PERF_EXEC mpiexec -n ${procs} $SIM_EXEC $SIM_EXEC_DEF_PARAMS $ADDL_SIM_CMD_LINE_ARGS $args > \${outdir}/sim_out_\${REP}.txt 2> \${outdir}/stats_out_\${REP}.txt"
    
	echo "#!/bin/bash"								  > "$script"
	echo "#PBS -N ${outDir}"                         >> "$script"
    echo "#PBS -l nodes=1:ppn=24"                    >> "$script"
    echo "#PBS -j oe"                                >> "$script"
    echo "cd \$PBS_O_WORKDIR"                        >> "$script"
    echo 'jobID=`echo ${PBS_JOBID} | cut -d"." -f1`' >> "$script"
    echo "outdir='$outDir'"                          >> "$script"
    echo 'mkdir -p ${outdir}'                        >> "$script"
    
	echo -e "\n\n\n\n"								 >> "$script"

	# Use helper script to set value for ST_REP
    setupInitRep "${script}" "${outdir}"
    echo "for REP in \`seq \${ST_REP} ${REPS}\`"     >> "$script"
    echo 'do'                                        >> "$script"
    echo "    $mainCmd"                              >> "$script"
    echo "    cat \"\${outdir}/stats_out_\${REP}.txt\" >> \"\${outdir}/sim_out_\${REP}.txt\"" >> "$script"
	echo "done"                                      >> "$script"
}

# The main function that coordinates tasks in this script.
function main() {
    # First process all command-line arguments and setup global variables
    parseParamsArgs $*
    if [ $? -ne 0 ]; then
	# Invalid parameters
	exit 1
    fi 
    # Print parameters and ranges for cross verifications
    printParamRanges
    # Now generate data various combination of parmaeters.
    initCmdLineParams
    while [[ 1 ]]
    do
        local cores=1
	
		# Setup the PBS script to be run 10 times
		local pbsScript=`toPathName`
        
		if [ $useMPI -eq 0 ]; then
        	echo "not using mpi"
		else
			local procs=${currCmdLineVals['procs']}
            	createMPIPBSscript "${pbsScript}.sh" "${pbsScript}" "${procs}"
		fi


		# Setup walltime based on core counts
        if [ ${cores} -lt 4 ]; then
            walltime="05:00:00"
        elif [ ${cores} -lt 20 ]; then
            walltime="03:00:00"
        else
            walltime="02:00:00"
        fi

		# For testing uncomment the following 2 lines
        # echo "Stopped for testing"
        # return 0

		# To check each PBS script, uncomment the following
        # less "${pbsScript}.sh"
        # echo "Continue (y/n)?: "
        # read goOn
        # if [ "${goOn}" != "y" ]; then
        #    return 0
        # fi

        # Submit the PBS job with the generated script & memory of 4 GB/core
        mem=$(( cores * 4 ))
        qsub -l "walltime=${walltime}" -l "mem=${mem}gb" "${pbsScript}.sh"

		# Generate next set of parameters
		nextParamComb
		if [ $? -ne 0 ]; then
	    	# Done exploring different parameter combinations.
	    	return 0
		fi
    done
}

# End of script
