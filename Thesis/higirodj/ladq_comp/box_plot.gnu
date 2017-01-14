# A gnuplot script to plot CSV data generated comparing percentage
# difference in run of various ladder queue configurations
#
# gnuplot -e box_plot.gnu
#

# Set properties for the chart.
dataFile="ph3_lq_compare.csv"
set datafile separator ","
set terminal pdfcairo enhanced color size 4in,2.5in
set output "lq_config_compare.pdf"

# Set properties for the box plot
set border 2 front linewidth 0.5
set boxwidth 0.5 absolute
set style fill solid 0.6
unset key
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit 
set xtics   ("L.List &\nList" 1.00000, "L.List &\nM.Set" 2.00000,\
             "L.List &\nHeap" 3.00000, "Vecor&\nM.Set"   4.00000,\
             "Vector &\nHeap" 5.00000, "Vecor&\nVector"  6.00000 ) font ",10" offset 0,0.5

set xrange[0:6]
set ytics border in nomirror font ",9"
set ytics  format "%2.0f%%"
set grid lc rgb "#cccccc"

set xlabel "Data structure for bucket and bottom" font " Bold,10"
set ylabel "Percentage performance difference\n(versus Vector-Vector)" font " Bold, 10"

# Set colors to be used
ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"


# Convenience function to run KS test and get 1 line result.
getKScmd(year) = "Rscript ../ks_test.R " . getRefData(year) . " " . getMsData(year) . " " . col . ' | grep "^D = "'
runKStest(year) = system(getKScmd(year))

# Plot box plot for each combination of values.

plot dataFile using (1):($16*100) lt 1 pt 7 lc rgb word(ColorList, 1),\
     dataFile using (2):($17*100) lt 1 pt 7 lc rgb word(ColorList, 2),\
     dataFile using (3):($18*100) lt 1 pt 7 lc rgb word(ColorList, 3),\
     dataFile using (4):($19*100) lt 1 pt 7 lc rgb word(ColorList, 4),\
     dataFile using (5):($20*100) lt 1 pt 7 lc rgb word(ColorList, 5),\
     dataFile using (6):($21*100) lt 1 pt 7 lc rgb word(ColorList, 6)
     
# end of script

# > x <- read.csv("ref_inf_peak_year1.csv", header=F)$V3
# > y <- read.csv("inf_peak_year1.csv", header=F)$V3
# > ks.test(y,x)
