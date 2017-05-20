# Gnuplot script to plot line graphs of peak memory use for different
# queues based on events-per agent.
#
# Run this script using the following command-line
#
# $ gnuplot -e 'config="ph3"' -e 'keyOn=1' plot_cache_comp.gnu

# Set input / output file names
csvDir = config . "_perf_data"
delay  = "10"
selfEvents = "0.250"
outFile1 = sprintf("%s_cache_miss_per_event_delay_%s.pdf", config, delay)
outFile2 = sprintf("%s_cache_miss_per_instr_delay_%s.pdf", config, delay)
outFile3 = sprintf("%s_tot_instrs_delay_%s.pdf", config, delay)
outFile4 = sprintf("%s_instrs_per_event_delay_%s.pdf", config, delay)

# Set the grep search string based on delay & self-Events values.
searchStr = sprintf("^ %s, %s, ", delay, selfEvents)

# Set properties for the chart.
set datafile separator ","
set terminal pdfcairo enhanced color size 3in,2.5in font ", 10"

# Set properties for the line plot
# set border 10 front linewidth 0.5
set style fill transparent solid 0.35

if (exists("keyOn")) {
    # set key at graph 0.60, 0.95 maxcols 1 samplen 1 font ",8"
    set key top left maxcols 1 samplen 1 font ",8"
} else {
    unset key
}
    
set xrange[0:21]
set xtics (1, 5, 10, 15, 20)
set grid lc rgb "#dddddd"

set xlabel "Events/LP" font " Bold,8" offset 0,0.5

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"

# Convenience functions to extract data to be plotted from CSV file(s).
getCsvFile(queue) = sprintf("%s/%s_%s_runtime_stats.csv", csvDir, config, queue)
# The following getData method remaps the following 3 blocks of information:
#   Committed Events     51-59    4-12
#   Cache misses         69-77   10-21
#   Instructions        105-113  22-30
getData(queue) = sprintf("< cut -d',' -f3-5,51-59,69-77,105-113 %s | grep '%s'", getCsvFile(queue), searchStr)

# Set queue / file names and color index for each queue to keep colors
# consistent throughout
queueList  = "heap 3tHeap 2tLadderQ ladderQ"
queueColor = "8 2 5 6"

# queueList  = "heap 3tHeap 2tLadderQ ladderQ 2tHeap fibHeap"
# queueColor = "8 2 5 6 3 4"

# place queue names at the right-corner to ease identification of curves
# do for [i=1:3] {
#     queue = word(queueList, i)
#     qClr  = word(ColorList, word(queueColor, i) + 0)
#     stats getData(queue) using 4 name 'runtime' nooutput
#     set label queue at 20.5, runtime_max center rotate by 90 font ",6" tc rgb qClr
# }

# Set label based on config at bottom center of graph
# set label config at graph 0.5, 0.1 center font " Bold,10"
# set label config . " (sequential)" at graph 0.5, 0.05 center font " Bold,10"

# Plot line charts for each queue of values.
#  for [i=1:4] getData(word(queueList, i)) using 3:($15*100/$6):($16*100/$7) with filledcurves lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\

# Plot chart showing cache miss per event 
set output outFile1
set ylabel "Cache misses per Event" font " Bold, 8" offset 2,0
set y2label "Total #Events (in millions)" font " Bold, 8" tc rgb "#7e2f8e" offset -1.5, 0
set y2tics tc rgb "#7e2f8e"
# set yrange [0:8]
set ytics nomirror

print getData(word(queueList, 1))

plot for [i=1:4] getData(word(queueList, i)) using 3:($13/$4):($15/$4):($16/$4) with yerrorbars lw 1 ps 0.2 pt 7 lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\
     for [i=1:4] getData(word(queueList, i)) using 3:($13/$4) with lines lw 2 lc rgb word(ColorList, word(queueColor, i) + 0) title word(queueList, i),\
     getData(word(queueList, 1)) using 3:($4/1e6) axes x1y2 with lines lw 2 lc rgb "#7e2f8e" title "Tot.Evts"

#--------------------------------------------------------------
     
# Plot chart showing cache miss per instruction
set terminal pdfcairo enhanced color size 2in,2.5in font ", 10"     
set output outFile2
set ylabel "Cache misses per Instruction" font " Bold, 8" offset 2,0
set yrange [0:*]
unset y2label
unset y2tics
set ytics mirror
set key top right

plot for [i=1:4] getData(word(queueList, i)) using 3:($13/$22):($15/$22):($16/$22) with yerrorbars lw 1 ps 0.2 pt 7 lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\
     for [i=1:4] getData(word(queueList, i)) using 3:($13/$22) with lines lw 2 lc rgb word(ColorList, word(queueColor, i) + 0) title word(queueList, i)

#--------------------------------------------------------------
     
# Plot chart showing instructions per event in the program
set terminal pdfcairo enhanced color size 2in,2.5in font ", 10"     
set output outFile4
set ylabel "Instructions per event" font " Bold, 8" offset 2,0
unset y2label
set yrange [*:*]
unset y2tics
set ytics mirror
set key top right

plot for [i=1:4] getData(word(queueList, i)) using 3:($22/$4):($24/$4):($25/$4) with yerrorbars lw 1 ps 0.2 pt 7 lc rgb word(ColorList, word(queueColor, i) + 0) notitle,\
     for [i=1:4] getData(word(queueList, i)) using 3:($22/$4) with lines lw 2 lc rgb word(ColorList, word(queueColor, i) + 0) title word(queueList, i)     

# End of script
