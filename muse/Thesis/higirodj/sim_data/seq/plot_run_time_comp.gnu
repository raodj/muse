# Gnuplot script to plot line graphs of simulation run times for
# different queues based on events-per agent.
#
# Run this script using the following command-line
#
# $ gnuplot -e 'config="ph3"' -e 'keyOn=1' plot_run_time_comp

# Set input / output file names
csvDir  = config
outFile = config . "_run_time.pdf"

# Set the grep search string based on delay & self-Events values.
delay = "1"
selfEvents = "0.250"
searchStr = sprintf("^ %s, %s, ", delay, selfEvents)

# Set properties for the chart.
set datafile separator ","
set terminal pdfcairo enhanced color size 1.75in,1.75in font ", 10"
set output outFile

# Set properties for the line plot
# set border 10 front linewidth 0.5
set style fill transparent solid 0.35

if (exists("keyOn")) {
    set key at graph 0.55, 0.95 maxcols 1 samplen 1 font ",8"
} else {
    unset key
}
    
set xrange[1:20]
set xtics (1, 5, 10, 15, 20)
set grid lc rgb "#dddddd"

set xlabel "Events/LP" font " Bold,8" offset 0,0.5
set ylabel "Run time (sec)" font " Bold, 8" offset 1,0

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"

# Convenience functions to extract data to be plotted from CSV file(s)
getCsvFile(queue) = sprintf("%s/%s_%s_runtime_stats.csv", csvDir, config, queue)
getData(queue) = sprintf("< cut -d',' -f3-9 %s | grep '%s'", getCsvFile(queue), searchStr)

# Set queue / file names and color index for each queue to keep colors
# consistent throughout
queueList  = "heap 3tHeap 2tLadderQ ladderQ"
queueColor = "8 2 5 6"

# place queue names at the right-corner to ease identification of curves
# do for [i=1:3] {
#     queue = word(queueList, i)
#     qClr  = word(ColorList, word(queueColor, i) + 0)
#     stats getData(queue) using 4 name 'runtime' nooutput
#     set label queue at 20.5, runtime_max center rotate by 90 font ",6" tc rgb qClr
# }

# Plot line charts for each queue of values.
plot for [i=1:4] getData(word(queueList, i)) using 3:6:7 with filledcurves lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\
     for [i=1:4] getData(word(queueList, i)) using 3:4:6:7 with yerrorbars lw 1 ps 0.2 pt 7 lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\
     for [i=1:4] getData(word(queueList, i)) using 3:4 with lines lw 2 lc rgb word(ColorList, word(queueColor, i) + 0) title word(queueList, i)

# End of script
