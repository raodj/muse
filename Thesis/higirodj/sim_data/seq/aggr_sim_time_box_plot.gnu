# A gnuplot script to plot manually aggreated simulation time data for
# ph3, ph4, and ph5 for the 6 different scheduler queues with
# different parameter settings
# 
# $ gnuplot aggr_sim_time_box_plot.gnu
#

# Set properties for the chart.
dataFile="AggrSimTimes.csv"
set datafile separator ","
set terminal pdfcairo enhanced color size 2.5in,1.5in font ", 10"
set output "AggrSimTimes_3tHeap_compare.pdf"

# Set properties for the box plot
set border 10 front linewidth 0.5
set boxwidth 0.5 absolute
set style fill solid 0.6
unset key
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit
set xtics   ("Heap"      1, "2tHeap"   2,\
             "fibHeap"   3, "3tHeap"   4) font ",8" offset 0,0.3


set xrange[0.5:4.5]
#set yrange[-60:110]
# set ytics (0, -20, -40, -60, -80, -100) border in nomirror font ",9"
set ytics  format "%2.0f%%" nomirror
set grid lc rgb "#cccccc"
set xzeroaxis

set xlabel "Scheduler queue data structure" font " Bold,8" offset 0,0.5
set ylabel "% Diff. in run time\nvs. 3tHeap" font " Bold, 8" offset 1,0

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"

# Set some labels to make chart data easier to interpret
red='#d95319'
set label 1 "Slower than\n3tHeap" at 4.4, -30 center font ',8' tc rgb red rotate by 90 front
set label 2 "Faster than\n3tHeap" at 4.4,  35 center font ',8' tc rgb red rotate by 90 front
set arrow 3 from graph 0.97,0 to graph 1,0 nohead
set arrow 4 from graph 0.97,1 to graph 1,1 nohead

# Plot box plot for each queue of values.
plot dataFile using (1):(($14  - $5) * 100 / $5) lt 1 pt 7 lc rgb word(ColorList, 1),\
     dataFile using (2):(($14  - $8) * 100 / $8) lt 1 pt 7 lc rgb word(ColorList, 2),\
     dataFile using (3):(($14 - $11) * 100 / $11) lt 1 pt 7 lc rgb word(ColorList, 3),\
     dataFile using (4):(($14 - $14) * 100 / $14) lt 1 lw 3 pt 7 lc rgb word(ColorList, 4)

# A second plot to compare simulation times versus the ladderQ
set output "AggrSimTimes_ladderQ_compare.pdf"
set xlabel "Scheduler queue data structure" font " Bold,8" offset -3,0.5

set xtics   ("3tHeap"    1, "2tLadderQ"  2,\
             "ladderQ"   3) font ",8" offset 0,0.3

# Set some labels to make interpretation of data easier
set label 1 "Slower than\nladderQ" at 4, -40 center font ',8' tc rgb red rotate by 90
set label 2 "Faster than\nladderQ" at 4,  40 center font ',8' tc rgb red rotate by 90
set arrow 1 from 3.75, -2 to 3.75, -40 lc rgb red head filled size 0.15,10
set arrow 2 from 3.75, +2 to 3.75, +40 lc rgb red head filled size 0.15,10

plot dataFile using (1):(($20 - $14) * 100 / $14) lt 1 pt 7 lc rgb word(ColorList, 4),\
     dataFile using (2):(($20 - $17) * 100 / $17) lt 1 pt 7 lc rgb word(ColorList, 5),\
     dataFile using (3):(($20 - $20) * 100 / $20) lt 1 lw 3 pt 7 lc rgb word(ColorList, 6)
             
# end of script
