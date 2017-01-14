# A gnuplot script to plot CSV data generated comparing percentage
# difference in run of various ladder queue configurations
#
# gnuplot -e box_plot_memory.gnu
#

# Set properties for the chart.
dataFile="ph3_lq_memory_compare.csv"
set datafile separator ","
set terminal pdfcairo enhanced color size 4in,2.5in
set output "lq_config_memory_compare.pdf"

# Color y-axis labels to help them standout.
blue="#0072bd"
# Set properties for the box plot
set border 2 front linewidth 0.5
set boxwidth 0.5 absolute
set style fill solid 0.6
unset key
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"
set ytics border in nomirror
set grid lc rgb "#cccccc"

# Convenience function to run KS test and get 1 line result.
getKScmd(year) = "Rscript ../ks_test.R " . getRefData(year) . " " . getMsData(year) . " " . col . ' | grep "^D = "'
runKStest(year) = system(getKScmd(year))

set multiplot

# Plot sub-plot with just one bar chart to use different scale for y-axis
set origin 0, 0
set size 0.3, 1

set xtics   ("L.List &\nList" 1.00000 ) font ",10" offset 0,0.5
set xrange[0:1]
set ytics  format "%2.0f{/Symbol \264}" font ",9"

# set xlabel "Data structure for bucket and bottom" font " Bold,10"
set ylabel "Percentage difference in memory use\n(versus Vector-Vector)" font " Bold, 10" offset 2,0
set xlabel " "

# Plot box plot for each combination of values.

plot dataFile using (1):($16*-1) lt 1 pt 7 lc rgb word(ColorList, 1)

# Draw second plot with different scale to show data better
set origin 0.23, 0
set size 0.76, 1

set xrange[1:6]
set xtics   ("L.List &\nM.Set" 2.00000,\
             "L.List &\nHeap" 3.00000, "Vecor&\nM.Set"   4.00000,\
             "Vector &\nHeap" 5.00000, "Vecor&\nVector"  6.00000 ) font ",10" offset 0,0.5

set ytics  format "%1.2f{/Symbol \264}"

set xlabel "Data structure for bucket and bottom" font " Bold,10" offset -10,0
unset ylabel

plot dataFile using (2):($17*-1) lt 1 pt 7 lc rgb word(ColorList, 2),\
     dataFile using (3):($18*-1) lt 1 pt 7 lc rgb word(ColorList, 3),\
     dataFile using (4):($19*-1) lt 1 pt 7 lc rgb word(ColorList, 4),\
     dataFile using (5):($20*-1) lt 1 pt 7 lc rgb word(ColorList, 5),\
     dataFile using (6):($21*-1) lt 1 pt 7 lc rgb word(ColorList, 6)
     
# End the multiplot environment to actually get the PDF written
unset multiplot

# end of script

# > x <- read.csv("ref_inf_peak_year1.csv", header=F)$V3
# > y <- read.csv("inf_peak_year1.csv", header=F)$V3
# > ks.test(y,x)
