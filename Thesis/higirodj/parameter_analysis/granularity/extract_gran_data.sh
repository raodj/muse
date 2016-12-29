#!/bin/bash

# Convenience script to extract granularity information from callgrind
# output files.

# The number of instructions spent in the body of executeTask
pholdExecTaskInstr=773559533

function printLog() {
	local file="$1"
	local totInstrs=$2
	local calls=$3
	# Now find total instructions in the program as a whole
	progInstrs=`grep "summary: " $file | cut -d" " -f2`
	# Now compute percent of run time
	totInstrs=$(( totInstrs + pholdExecTaskInstr ))
	percent=`echo "$totInstrs * 100 / $progInstrs" | bc -l`
	# Round it to 2 decimals
	percent=`printf %.2f $percent`
	# Compute instructions per event
	instrPerEvt=$(( totInstrs / calls ))
	# Print log entry
	echo "$gran, $percent, $instrPerEvt"
}

for gran in `seq 0 50`
do
	# Check if there is a callgrind output file with this granularity value
	file=`grep "granularity $gran " callgrind.out.* | cut -d":" -f1`
	if [ $? -eq 0 ]; then
		# Process the granularity data
		startLine=`grep -n "simGranularity" $file | cut -d":" -f1`
		endLine=$(( startLine + 8 ))		
		# Ensure the endline is as expected
		endEntry=`head -${endLine} $file | tail -1`
		if [ "$endEntry" != "+6 7492500" ]; then
			# This could be in alternate format used by callgrind
			secondLine=$(( startLine + 1 ))
			lineEntry=( `head -${secondLine} $file | tail -1 | tr "=" " "` )
			if [ ${lineEntry[0]} == "calls" ]; then
				# This is alternative format.
				endLine=$(( startLine + 6 ))
				totInstrs=`head -${endLine} $file | tail -6 | grep "^*" | cut -d" " -f2`
				# Print the results
				printLog "$file" $totInstrs ${lineEntry[1]}
			else
				echo "end entry is: $endEntry"
				echo "Unexpected end entry in $file. Skipping it."
			fi
		else
			# Extract the instruction counts in the previous 8 lines
			instrCounts=`head -${endLine} $file | tail -8 | cut -d" " -f2 | tr "\n" "+"`
			# Sum them up!
			totInstrs=`echo ${instrCounts}0 | bc`
			# Print the results
			events=( $endEntry )
			printLog "$file" $totInstrs ${events[1]}
		fi
	else
		echo "No matching callgrind output file for granularity $gran"
	fi 
done

# End of script
