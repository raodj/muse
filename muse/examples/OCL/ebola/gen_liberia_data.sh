#!/bin/bash

# Script to reproduce the results reported by Rivers et. al
# http://currents.plos.org/outbreaks/article/obk-14-0043-modeling-the-impact-of-interventions-on-an-epidemic-of-ebola-in-sierra-leone-and-liberia/#ref10
# This script performs the following tasks:
#
#  1. Generates a simulation via EDL parser using ebola.edl
#  2. Runs ODE & SSA simulations
#  3. Plots a chart.
#

# Helper function to generate the model and compile it.  This function
# is called from the main function only if the executable does not
# already exist.
function generateModel() {
    # Generate a simulation from ebola.edl
    echo "Generating simulation from ebola.edl..."
    ../edl/edl_parser ebola.edl
    if [ $? -ne 0 ]; then
        echo "Error generating model."
        exit 1
    fi
    echo "Model generating successfully. Compiling the model..."
    make --silent
    if [ $? -ne 0 ]; then
        echo "Error compiling the generated model."
        exit 1
    fi
    echo "Model compiled successfully."
}

# Helper function to run the generated exeutable to produce ODE and
# SSA values.
function simulate() {
    # Generate ODE version upto 175 days as per the paper.
    ./EbolaLiberia --initVals 470000 3 0  --rows 1 --cols 1 --simEndTime 176 --step 0.001 | grep "Agent" > liberia_ode.csv
    if [ $? -ne 0 ]; then
        # Simulation did not run successfully.
        exit 3
    fi
    # Use the Exposed and Infective entries in the last line of the
    # ODE file to generate 250 SSA replications
    day175=( `tail -1 "liberia_ode.csv" | tr "," " "` )
    # Truncate doubles to whole numbers.
    sus=`echo ${day175[3]} | cut -d"." -f1`
    exp=`echo ${day175[4]} | cut -d"." -f1`
    inf=`echo ${day175[5]} | cut -d"." -f1`
    echo "Running SSA with ODE values of: sus=${sus}, exp=${exp}, inf=${inf}"

    # Run 250 ssa replications
    rm -f liberia_ssa.csv
    for rep in `seq 1 250`
    do
        echo -n -e "Running SSA rep: ${rep}    ...\r"
        ./EbolaLiberia --ssa --initVals $sus $exp $inf  --rows 1 --cols 1 --simEndTime 100 --step 0.001 | grep "Agent" >> liberia_ssa.csv
        if [ $? -ne 0 ]; then
            # Simulation did not run successfully.
            exit 4
        fi
        # Add an empty line to generate plots correctly
        echo >> liberia_ssa.csv
    done
    # Add a new line
    echo
}

# Helper function to remove all the generated intermediate files.
function cleanup() {
    echo "Cleaning up generated files."
    rm -f *.h *.cpp EbolaLiberia liberia_ode.csv liberia_ssa.csv\
       vv_liberia.pdf OclEqnSolvers.c OclEqnSolvers.c_inc
}

# The top-level main function that coordinates the various tasks.
function main() {
    # If a command-line argument is supplied then honor that.
    if [ "$1" == "clean" ]; then
        cleanup
    else
        if [ ! -x "EbolaLiberia" ]; then
            # Executable does not exist. Generate it
            generateModel
            if [ $? -ne 0 ]; then
                # Model generation or compilation failed.
                exit 2
            fi
        fi
        # Now that the executable exists, run it and plot the chart
        simulate
        if [ $? -ne 0 ]; then
            # Simulations did not run successfully.
            echo "Simulations did not finish successfully"
            exit 3
        fi
        # Plot the chart
        echo "Plot a chart using generated simulation data."
        gnuplot plot_data.gp
        if [ $? -ne 0 ]; then
            echo "Gnuplot did not complete successfully"
            exit 4
        fi
        # View the generated PDF
        evince "vv_liberia.pdf"
    fi
}

# Let the main function do the tasks
main $*

# End of script
