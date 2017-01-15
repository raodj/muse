# Plots the chart showing the impact of varying max_rungs in a PHOLD
# simulation.  The data in max_rungs_data.csv was collected with the
# following command line on a compute note on redhawk:

# for i in `seq 1 10`; do time ./phold --rows 100 --cols 10 --eventsPerAgent 4 --delay 1 --simEndTime 5000 --scheduler-queue ladderQ --lq-max-rungs 4 | grep -i "bottom" ; done

dataFile = 'max_rungs_data.csv'
set datafile separator ','
set terminal pdfcairo enhanced color size 3.5in,2.25in
set output 'max_rung_effect.pdf'

#set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.075
# set style fill transparent solid 0.3 noborder
#unset key
#set pointsize 0.5
#set style data boxplot
#set xtics border in scale 0,0 nomirror norotate  offset character 0, 0, 0 autojustify
#set xtics  norangelimit
#set xtics   ()
#set ytics border in scale 1,0.5 nomirror norotate  offset character 0, 0, 0 autojustify
# set title "Configuration: 1 node, 12 processes/node" font " Bold"
set ylabel "Simulation runtime (seconds)" font " Bold"
set xlabel "Max rungs in Ladder Queue" font " Bold"

# set label "Log scale" at 500, 65.5 center font " Bold"
set xrange[1:12]
#set yrange[65:72]

plot dataFile using 1:($3-$4):($3+$4) with filledcurves lc rgb "#ddffff" notitle, \
     dataFile using 1:($3-$4):($3+$4):3 with yerrorbars lt -1 notitle, \
     dataFile using 1:3 with lines lw 1 lt -1 lc rgb "#0000ff" title "Runtime (sec)"
 
# End of script
 
