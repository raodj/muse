# A gnuplot script to plot CSV data to compare heap and binomial heap
# runtimes.  Run this script as:
#
# gnuplot -e box_plot.gnu

# Set properties for the chart.
dataFile="full_gsa.csv"
set datafile separator ","
set terminal pdfcairo enhanced color size 3.5in,3in
set output "heap_binomHeap_compare.pdf"

# Set properties for the box plot
set border 2 front linewidth 0.5
set boxwidth 0.5 absolute
unset key
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit
set xtics   ("Binary\nHeap" 0.5,\
             "Binomial\nHeap" 2.0,\
             "Runtime\nDifference" 3.5) font ",10" offset 0,0.3

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"
             
set style fill solid 0.6
set xrange[0:4]
# set yrange[-100:0]
# set ytics (0, -20, -40, -60, -80, -100) border in nomirror font ",9"
set ytics 100 nomirror
set y2tics tc rgb word(ColorList, 7) format "%2.0f%%"
set grid lc rgb "#cccccc"

set xlabel "Scheduler data structure" font " Bold,10"
set ylabel "Runtimes (sec)" font " Bold, 10" offset 1,0
set y2label "%Runtime difference\n(Heap vs. Binomial heap)" font " Bold, 10" tc rgb word(ColorList, 7)

set arrow from 4,0 to 4,500 nohead lc rgb word(ColorList, 7)
do for [per=-5:50:5] {
  set arrow from second 4,per to second 3.9,per nohead lc rgb word(ColorList, 7)
}

# Plot box plot for each combination of values.

plot dataFile using (0.5):12 lt 1 pt 7 lc rgb word(ColorList, 1),\
     dataFile using (2):13 lt 1 pt 7 lc rgb word(ColorList, 2),\
     dataFile using (3.5):(($13-$12)*100/$13) axes x1y2 lt 1 pt 7 lc rgb word(ColorList, 7)
     
# end of script
