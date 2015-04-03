#!/bin/bash
#PBS -S /bin/bash

# When running with qsub use the following command:
# qsub -N "fibVsLad_5delay" -v "DELAY=5" -v "GRANULARITY=0" ./fibHeapVsLadderQ.sh

# Global variable to set number of samples for averaging
samples=5

# Change to PHOLD directory for running tests
if [ ! -z "$PBS_O_WORKDIR" ]; then
    cd "$PBS_O_WORKDIR"
fi

# Output data/log file
dataFile="fibVsLadTimes.dat"

# Temporary output file
outFile="output.txt"
timeFile="timing.txt"

# Function to measure phold runtime with given number of agents,
# delay, and queue. The parameters to this function are:
# $1: Rows (which determines number of agents)
# $2: Number of events per agent
# $3: Random delay added to events
# $4: Scheduler queue (fibHeap or ladderQ)
# $5: The granularity for the events
#
function getTime() {
    local rows=$1
    local events=$2
    local delay=$3
    local queue=$4
    local granularity=$5

    # Clear out old timing file
    rm -f "$timeFile" "$outFile"
    # Compute end time to ensure sim is not too long
    local endTime=$(( 5000000 / $rows / $events ))
    for i in `seq 1 $samples`
    do
        # Run the simulation
        /usr/bin/time -p -a -o "$timeFile" ./phold --rows $rows --cols 1 --eventsPerAgent $events --delay $delay --scheduler-queue $queue --granularity $granularity --simEndTime $endTime >> "$outFile"
    done
    # Extract sample timings from file and compute average
    local times=`grep "real" "$timeFile" | cut -d" " -f2 | tr "\n" "+"`
    times="(${times}0)/$samples"
    local avg=`echo -e "scale=5\n$times" | bc -l`
    echo $avg
}

# Pass the search string as the first parameter. 
function getAvgStats() {
    local avg=$1
    local srchTerm="$2"
    if [ ! -e "$outFile" ]; then
        echo "Output file $outFile was not found!"
        exit 2
    fi
    # Extract sammple values for the search term from output file
    local values=""
    if [ $avg -eq 0 ]; then
        # For non-averages get term right after ":"
        values=`grep "$srchTerm" $outFile | cut -d":" -f2 | tr "\n" "+"`
    else
        # For averages get term right after "="
        values=`grep "$srchTerm" $outFile | cut -d"=" -f2 | tr "\n" "+"`
    fi
    local plusCnt="${values//[^+]}"
    if [ ${#plusCnt} -ne $samples ]; then
        echo "Did not find $samples values for '$srchTerm' in '$outFile'"
        echo "Line extracted: $values"
        exit 3
    fi
    # Compute averages
    values="(${values}0)/$samples"
    local avg=`echo -e "scale=5\n$values" | bc -l`
    echo $avg
}

# Convenience function to extract averages for various ladderQ stats
function getLadderStats() {
    local topIns=`getAvgStats 0 "Inserts into top"`
    local ladIns=`getAvgStats 0 "Inserts into rungs"`
    local botIns=`getAvgStats 0 "Inserts into bottom"`
    local rungCnt=`getAvgStats 0 "Max rung count"`
    local avgBkts=`getAvgStats 1 "Average bucket count"`
    local avgBotSz=`getAvgStats 1 "Average bottom size"`
    
    # Print the information for csv
    echo "$topIns, $ladIns, $botIns, $rungCnt, $avgBkts, $avgBotSz"
}

# Pass delay as the first parameter
# Pass granularity as the second parameter
function generateTimeData() {
    local delay=$1
    local granularity=$2
    local row=1
    # Print header for reference
    local header="# Num.Agents, EventsPerAgent, Delay, LadderQTime, FibHeapTime,TopIns, LadIns, BotIns, MaxRungs, AvgBuckets, AvgBotSize, Schedules, TotalEvents, Granularity, HeapTime"
    local ladderTime="na"
    local fibTime="na"
    local heapTime="na"
    local ladderStats="na, na, na, na, na, na"
    echo "$header"
    echo "$header" >> "$dataFile"
    # Generate data
    while [ $row -le 20000 ];
    do
        for events in `seq 1 10`
        do
            echo "Running rows=$row, events=$events"
            # ladderTime=`getTime $row $events $delay ladderQ $granularity`
            # ladderStats=`getLadderStats`
            # fibTime=`getTime $row $events $delay fibHeap $granularity`
            heapTime=`getTime $row $events $delay heap $granularity`
            local totSched=`getAvgStats 0 "Total schedules"`
            local totEvents=`getAvgStats 0 "Total Scheduled Events"`
            echo $row, $events, $delay, $ladderTime, $fibTime, $ladderStats, $totSched, $totEvents, $granularity, $heapTime >> "$dataFile"
            echo $row, $events, $delay, $ladderTime, $fibTime, $ladderStats, $totSched, $totEvents, $granularity, $heapTime
        done
        if [ $row -gt 1000 ]; then
            row=$(( row + 2500 ))
        elif [ $row -eq 1000 ]; then
            row=2500
        else
            row=$(( row * 10 ))
        fi
    done
}

# Convenience function to just run 1 test
function runOne() {
    local row=$1
    local events=$2
    local delay=$3
    local granularity=$4
    local col=1
    
    echo "Running rows=$row, events=$events, delay=$delay, granularity=$granularity, samples=$samples"
    local ladderTime=`getTime $row $events $delay ladderQ $granularity`
    local ladderStats=`getLadderStats`
    local fibTime=`getTime $row $events $delay fibHeap $granularity`
    echo $row, $events, $delay, $ladderTime, $fibTime, $ladderStats, $granularity
}

# The first parameter is assumed to be delay
function main() {
    if [ $# -eq 4 ]; then
        runOne $*
        exit 0
    fi
    if [ $# -ne 2 -a -z "$DELAY" -a -z "$GRANULARITY" ]; then
        echo "Specify delay and granularity as the only parameters"
        exit 3
    fi
    local delay=0
    if [ ! -z "$DELAY" ]; then
        delay=$DELAY
    else
        delay=$1
    fi
    local granularity=0
    if [ ! -z "$GRANULARITY" ]; then
        granularity=$GRANULARITY
    else
        granularity=$2
    fi

    dataFile="fibVsLadTimes_delay_$delay.dat"
    outFile="output_delay_$delay.txt"
    timeFile="timing_delay_$delay.txt"
    generateTimeData $delay $granularity
}

# The main part of the script
main $*

# end of script
