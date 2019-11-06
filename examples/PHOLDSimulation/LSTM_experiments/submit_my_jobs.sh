#!/bin/bash

# This is a shell script that must be run as a standalone script.  It
# is very straightforward and submits multiple PBS jobs to run 10
# iterations of simulations.  Run as (with parameters to simulator at
# the end):
#
# $ ./submit_my_jobs.sh --auto-mt-sub-queues --params '--rows 100 --cols 100 --scheduler-queue ladderQ --simEndTime 500' > ph4_ladderQ_4proc_log.txt 2>&1 &

# The list of command-line parameters to be varied for simulation.
# These values change from simulation-to-simulation.  Entries are in
# the form: cmd-line-arg="values".  Arguments with a '%' sign at
# the end are converted to percentage values in the range 0.0 to 1.0

if [ -z ${CmdLineParams+x} ]; then 
    # The following configuration was used for simulations for
    # recording runtime charachteristics.
    declare -A CmdLineParams
    CmdLineParams=( ['--recvr-range']="10 100 1000 10000"
        #           ['--threads-per-node']="1 2 4 8 14 20 24 28"
        #           ['--threads-per-node']="28"    
        #           ['procs']="1 2 4 8 14 20 24 28"
        #           ['procs']="1 2 4 8 12 14 18"
                    ['procs']="1 2 4"
        #           ['procs']="28"
        #           ['--eventsPerAgent']="1 2 5 10 15 20"
        #           ['--poll']="always avg exp lstm"
    )
fi

# Include some standard command-line options and helper functions
sourceDir="$(dirname "$0")"
source "${sourceDir}/my_jobs_funcs.sh"

# ------ Typically you should not have to modify values below this line. -----

# Run the main function with parameters

main $*

# End of script
