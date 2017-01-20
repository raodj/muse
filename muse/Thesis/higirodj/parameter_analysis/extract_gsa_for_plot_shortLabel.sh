#!/bin/bash

# This is a convenience script that is used to prepare Sobol indexes
# (and associated 95% CI) from error logs of a sensitivity analysis.
# This script is typically invoked from plot_gsa_anal.gnu.

# This script is the same as extract_gsa_for_plot.sh but it generates
# shorter labels for x-axis to make charts for PADS paper fit better
# in the paper's layout.

if [ $# -ne 2 ]; then
    >&2 echo "Specify the following 3 parameters:"
    >&2 echo "    1. Suffix for output GSA CSV files"
    >&2 echo "    2. Output file."
    >&2 echo " Example: ./extract_gsa_for_plot.sh ./gsa1 gsa_summary.csv"
    exit 1
fi

# Setup aliases for parameters to keep script readable
suffix="$1"
output="$2"

# The following column names are used for analysis 
colNames=( "GVTPeriod" "Imbalance" "Lambda" "SelfEvents%" "SimEndTime"
           "Rows" "EventsPerAgent" "Granularity" "Cols" )

# Setup associative array to map long EpiParams names to acronyms
declare -A Acronym
Acronym=( ["GVTPeriod"]="GVT"
          ["Imbalance"]="Imba-\nlance"
          ["Lambda"]="{/Symbol l}"
          ["SelfEvents%"]="%Self\nEvt."
          ["SimEndTime"]="End\nTime"
          ["Rows"]="Rows"
          ["EventsPerAgent"]="Evt./\nLP"
          ["Granularity"]="Evt.\nGran."
          ["Cols"]="Cols"
        )

# Print header for the output from this script
echo "# idx, name, abbr, f-measure, 95%_ci, f_round, ci_round" > "$output"

# Print the data for each parameter
MaxCols=$(( ${#colNames[@]} - 1 ))
idx=0
for col in `seq 0 $MaxCols`
do
    # Map column name to its acronym
    name="${colNames[$col]}"
    abbr="${Acronym[$name]}"
    if [ -z "$abbr" ]; then
        >&2 echo "No acronym for $name ?"
    fi
    gsaCSV="${suffix}_${name}_gsa.csv"
    f=0
    ci=0
    if [ ! -f "$gsaCSV" ]; then
        >&2 echo "Unable to find $gsaCSV"
    else
        gsaVals=( `tail -1 "$gsaCSV"` )
        f=${gsaVals[4]}
        ci=${gsaVals[5]}
    fi
    # Round values to 3 decimals to print labels
    roundVal=`printf "%.3f" $f`
    roundErr=`printf "%.3f" $ci`
    echo "$idx, $name, $abbr, $f, $ci, $roundVal, $roundErr" >> "$output"
    idx=$(( idx + 1 ))
done

# End of script
