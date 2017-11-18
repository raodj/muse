#!/bin/bash

# A convenience script with different helper methods extract various
# parameters from the generated data files.  The methods in this file
# are used by the collage_stats.sh script file for a specific model.

# Convenience function to convert values in the form 1hour, 1hours,
# 1day, 7days, 24weeks into simply numerical value in number of hours
#   $1 = The value to be converted
# This method echos the converted value in hours
function toHours() {
    local val=`expr "$1" : '\([0-9]*\)'`
    local unit=`expr "$1" : '[0-9]*\([a-z]*\)'`
    if [ -n "$unit" ]; then
	# Have a unit. Map unit to a suitable scale (number of hours) Bash
	# 4+ has associative arrays but Red Hawk has old bash does not
	# support it. So the following for-loop is a backwards
	# compatibility solution.
	validUnits=( hour hours  day days week weeks )
	validScale=( 1     1     24  24   168  168 )
	for idx in `seq 0 5`; do
		if [ "${validUnits[$idx]}" == "$unit" ]; then
			val=$(( val * ${validScale[$idx]} ))
		fi
	done
    fi
    # Print the convered value.
    echo $val
}

# Function to compute mean and confidence interval The parameters to
# this function are assumed to be the values to be processed.
#   $*: The set of numbers whose mean and CI is to be computed.
#
function calcMeanAndCI() {
    local n=$#
	if [ $n -lt 1 ]; then
		>&2 echo "calcMeanAndCI called without any parameters"
		echo "0 0 0 0"
		return 1
	fi 
    local meanSD=( `echo "$*" | tr " " "\n" | awk 'function abs(v){return (v<0?-v:v);}{sum+=$1; sumsq+=$1*$1}END{print (sum/NR), sqrt(abs(sumsq/NR - (sum/NR)**2))}'` )
    # 95% CI two sided T values from http://en.wikipedia.org/wiki/Student%27s_t-distribution
    local twoSideT=( 12.71 4.303 3.182 2.776 2.571 2.447 2.365 2.306 2.262 2.228 2.201 2.179 2.160 2.145 2.131 2.120 2.110 2.101 2.093 2.086 )
    # The degrees of freedom is one less than number of samples.
    n=$((n - 1))
    # CI = mean +- twoSideT[n] * sd / sqrt(n)
    local ci=( `echo "${meanSD[0]} ${twoSideT[$n]} ${meanSD[1]} $#" | awk '{print ($1-($2 * $3 / sqrt($4))), ($1+($2 * $3 / sqrt($4)))}'` )
    echo ${meanSD[0]} ${meanSD[1]} ${ci[0]} ${ci[1]}
}

# Computes the median of the supplied numbers and echoes the median
# value. This function is meant to be called only from calcBoxVals (as
# it assumes the supplied numbers are already sorted)
function calcMedian() {
    local data=( $* )
    # Median for odd and event number of samples is a bit different.
    local isOdd=`expr ${#data[*]} % 2`
    local midPos=`expr ${#data[*]} / 2`
    local median=0
    if [ $isOdd -eq 1 ]; then
	median=${data[$midPos]}
    else
	local nxtPos=$((midPos + 1))
	median=`echo ${data[$midPos]} ${data[$nxtPos]} | awk '{print ($1 + $2) / 2}'`
    fi
    # Echo the median
    echo $median
}

# Use numbers supplied as parameters and compute data for a box plot
#   $* = The list of numbers to be used to compute box plot.
#
# This function prints the following stats minimum value, 1st
# quartlie, median, second quartile, maximum value.
#
function calcBoxVals() {
    # Compute list of values to operate on
    local data=( `echo "$*" | tr " " "\n" | sort -n` )
    # Compute central median
    local median=`calcMedian ${data[*]}`
    # Now compute ranges for lower and upper quartile
    local end1st=`expr ${#data[*]} / 2`
    local start2nd=$((end1st + 1))
    local isOdd=`expr ${#data[*]} % 2`
    if [ $isOdd -eq 0 ]; then
	start2nd=$((start2nd + 1))
    fi
    # Now compute the median for first and second quartile
    quartile1=( ${data[@]:0:$end1st} )
    quartile2=( ${data[@]:$start2nd} )    
    local median1st=`calcMedian ${quartile1[*]}`
    local median2nd=`calcMedian ${quartile2[*]}`
    local minVal=$data
    local maxVal=${data[${#data[@]} - 1]}
    # Print values for the box plots
    echo "$minVal $median1st $median $median2nd $maxVal"
}

# Summarizes data for a given job given the file name prefix.
#  $1 = Prefix of the files in pwd to be summarized
#  $2 = number of entries expected in each file.
#  $3 = x-axis value
#  $4 = summary data file
#  $5 = Search string to identify line of statistic
#  $6 = Column in search line containing numeric statistic
#  $7 = "one" or "sum" to select one or sum of values in this stats
function summarize_runtime_stats() {
    # Set variable names to make function more readable
    local prefix=$1
    local entries=$2
    local xaxis=$3
    local dataFile=$4
    local srchStr="$5"
    local statCol=$6
    local aggrType=$7
    # An array of "one" or "sum" statistics extracted from multiple files
    local modelStats=()
    local fileNum=1
    for outFile in `ls -1 ${prefix}*`; do
		statList=( `grep "$srchStr" ${outFile} | cut -d' ' -f${statCol} | sort -n` )
		if [ ${#statList[*]} -lt $entries ]; then
			>&2 echo "Unable to extract $2 entries from $outFile (extracted only ${#statList[*]} entries): '$srchStr'"
		fi
		# The at least one stat data was obtained when either select
		# one or sum the values depending on the parameter specified.
		if [ ${#statList[*]} -gt 0 ]; then
			if [ "${aggrType}" == "one" ]; then
				local midVal=`expr ${#statList[*]} / 2`
				modelStats[${fileNum}]=${statList[$midVal]}
			elif [ "${aggrType}" == "max" ]; then
				local maxIdx=$(( ${#statList[@]} - 1 ))
				modelStats[${fileNum}]=${statList[$maxIdx]}
			else
				# Use sum of values as the reference
				sum=`echo "${statList[*]}" | tr " " "\n" | awk '{sum+=$1;}END{print sum}'`
				modelStats[${fileNum}]=$sum
			fi
			fileNum=$((fileNum + 1))
		fi
    done
    if [ ${#modelStats[*]} -gt 0 ]; then
        # Now we have stats from all the files. Print summary values
        # for plotting averages and confidence intervals.
		echo "$xaxis ${#modelStats[*]} `calcMeanAndCI ${modelStats[*]}` `calcBoxVals ${modelStats[*]}`"
    else
		# Print warning message that data for given configuration is missing
		>&2 echo "Warning: Data not found for ${prefix}. Skipping entry"
    fi
}

# Summarizes wall clock time for a given job given the file name prefix.
#  $1 = Prefix of the files in pwd to be summarized
#  $2 = number of entries expected in each file.
#  $3 = x-axis value
#  $4 = summary data file
#  $5 = Search string to identify line of statistic -- ignored
#  $6 = Column in search line containing numeric statistic -- ignored
#  $7 = "one" or "sum" to select one or sum of values in this stats
function summarize_elapsed_time_stats() {
    # Set variable names to make function more readable
    local prefix=$1
    local entries=$2
    local xaxis=$3
    local dataFile=$4
    local srchStr="$5"
    local statCol=$6
    local aggrType=$7
    # An array of "one" or "sum" statistics extracted from multiple files
    local modelStats=()
    local fileNum=1
    for outFile in `ls -1 ${prefix}*`; do
		# Extract raw elapsed time in h:mm:ss or m:ss format
		local statList=( `grep "$srchStr" ${outFile} | cut -d' ' -f${statCol}` )
		
		if [ ${#statList[*]} -lt $entries ]; then
			>&2 echo "Unable to extract $2 entries from $outFile (extracted only ${#statList[*]} entries): '$srchStr'"
		fi
		# Normalize elapsed times to seconds
		local count=$(( ${#statList[@]} - 1 ))
		for idx in `seq 0 $count`
		do
			local wallTime=${statList[$Idx]}
			local fmt=( ${wallTime//:/\ } )
			local runTime=0
			if [ ${#fmt[@]} -eq 3 ]; then
				runTime=`echo "$wallTime" | awk -F: '{ print ($1 * 3600) + ($2 * 60) + $3 }'`
			else
				runTime=`echo "$wallTime" | awk -F: '{ print ($1 * 60) + $2 }'`
			fi
			statList[$count]=$runTime
		done
		# Now sort the statList as numbers for further operations
		statList=( `echo ${statList[@]} | sort -n` )
		# The at least one stat data was obtained when either select
		# one or sum the values depending on the parameter specified.
		if [ ${#statList[*]} -gt 0 ]; then
			if [ "${aggrType}" == "one" ]; then
				local midVal=`expr ${#statList[*]} / 2`
				modelStats[${fileNum}]=${statList[$midVal]}
			elif [ "${aggrType}" == "max" ]; then
				local maxIdx=$(( ${#statList[@]} - 1 ))
				modelStats[${fileNum}]=${statList[$maxIdx]}
			else
				# Use sum of values as the reference
				sum=`echo "${statList[*]}" | tr " " "\n" | awk '{sum+=$1;}END{print sum}'`
				modelStats[${fileNum}]=$sum
			fi
			fileNum=$((fileNum + 1))
		fi
    done
    if [ ${#modelStats[*]} -gt 0 ]; then
        # Now we have stats from all the files. Print summary values
        # for plotting averages and confidence intervals.
		echo "$xaxis ${#modelStats[*]} `calcMeanAndCI ${modelStats[*]}` `calcBoxVals ${modelStats[*]}`"
    else
		# Print warning message that data for given configuration is missing
		>&2 echo "Warning: Data not found for ${prefix}. Skipping entry"
    fi
}

# End of script
