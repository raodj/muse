# Plots the chart showing the impact of varying max_rungs in a PHOLD
# simulation.  The data in max_rungs_data.csv was collected with the
# following command line on a compute note on redhawk:

# for i in `seq 1 10`; do time ./phold --rows 100 --cols 10 --eventsPerAgent 4 --delay 1 --simEndTime 5000 --scheduler-queue ladderQ --lq-max-rungs 4 | grep -i "bottom" ; done

dataFile = 'max_rungs_data.csv'
set datafile separator ','
set terminal pdfcairo enhanced color size 3.5in,1.5in
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
set ylabel "Sequential runtime\n(seconds)" font " Bold" offset 1,-1
set xlabel "Max rungs in Ladder Queue" font " Bold"

set label "Suboptimal number\nof rungs. Several inserts\ninto bottom and\nmany Bottom to Ladder\noperations." at 1.5, graph 0.45 font ",8"

set style rect fillstyle solid 1.0 noborder
set object rect from 1, graph 0 to 6, graph 1 behind fc rgb '#fff5e6'

set style rect fc rgb '#e6ffe6' fs solid 1.0 noborder
set object rect from 6, graph 0 to 12, graph 1 behind fc rgb '#e6ffe6'

set arrow from 6, graph 0 to 6, graph 1 nohead lt 0
set arrow from 6, graph 0.8 to 6.5, graph 0.8
set label "Sufficient number of rungs.\nNo inserts into bottom.\nZero Bottom to Ladder operations.\nIdeal configuration for Ladder Queue" at 6.6, graph 0.88 font ",9"

set arrow from 8, 11 to 8, 8 lc rgb '#a2142f'
set label "Max rungs set to 8.\nAlso proposed by\nTang et. al" tc rgb '#a2142f' at 8, 21 font ",8" center

# set label "Log scale" at 500, 65.5 center font " Bold"
set xrange[1:12]
set yrange[5:45]
set ytics (5, 15, 25, 35, 45)
#set yrange[65:72]

plot dataFile using 1:($3-$4):($3+$4) with filledcurves lc rgb "#ddffff" notitle, \
     dataFile using 1:3:4 with yerrorbars lt -1 notitle, \
     dataFile using 1:3 with lines lw 1 lt -1 lc rgb "#0000ff" notitle
 
# End of script
 
