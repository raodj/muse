# A gnuplot script to plot CSV data generated comparing percentage
# difference in run of various ladder queue configurations
#
# gnuplot -e box_plot.gnu
#

# Set properties for the chart.
dataFile="ph3_lq_compare.csv"
set datafile separator ","
set terminal pdfcairo enhanced color size 4in,1.5in font "ZapfDingbats"
set output "lq_config_compare.pdf"

# Set properties for the box plot
set border 2 front linewidth 0.5
set boxwidth 0.5 absolute
unset key
set pointsize 0.5
set style data boxplot
set xtics border in scale 0,0 nomirror norotate  autojustify
set xtics  norangelimit
set xtics   ("1 L.List &\nL.List" 1.00000, "2 L.List &\nM.Set" 2.00000,\
             "3 L.List &\nHeap" 3.00000, "4 Vec.&\nM.Set"   4.00000,\
             "5 Vec. &\nHeap" 5.00000, "6 Vec.&\nVec."  6.00000 ) font ",10" offset 0,0.3

# For some reason special characters would not work correctly in
# labels. So faking them below:

set style fill solid 1.0
# Cirlce y coordinate depends on overall chart size
cy = 0.22
set label at screen 0.23, cy "1" center tc rgb '#ffffff' font " Bold,9"
set obj 1 circle at screen 0.23, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.361, cy "2" center tc rgb '#ffffff' font " Bold,9"
set obj 2 circle at screen 0.361, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.494, cy "3" center tc rgb '#ffffff' font " Bold,9"
set obj 3 circle at screen 0.494, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.632, cy "4" center tc rgb '#ffffff' font " Bold,9"
set obj 4 circle at screen 0.632, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.76, cy "5" center tc rgb '#ffffff' font " Bold,9"
set obj 5 circle at screen 0.76, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.895, cy "6" center tc rgb '#ffffff' font " Bold,9"
set obj 6 circle at screen 0.895, cy size 0.1 behind fc rgb '#000000'

set label at screen 0.6, 0.5 "L.List: Linked list (std::list)\nVec: Vector (std::vector)\nM.Set: std::multi\\_set" font ",10"

set style fill solid 0.6
set xrange[0:6]
set yrange[-100:0]
set ytics (0, -20, -40, -60, -80, -100) border in nomirror font ",9"
set ytics  format "%2.0f%%"
set grid lc rgb "#cccccc"

set xlabel "Data structure for bucket and bottom" font " Bold,10"
set ylabel "% diff in runtime\nvs. Vec-Vec" font " Bold, 10" offset 3,0

# Add additional labels to show slow-down to make percentages easier
# to interpret.
clr='#d95319'
set label "1{/Symbol \264}" at 0.15, 0 font ",8" tc rgb clr
set label "2{/Symbol \264}" at 0.15, -50 font ",8" tc rgb clr
set label "5{/Symbol \264}" at 0.15, -80 font ",8" tc rgb clr
set label "100{/Symbol \264}" at 0.15, -99 font ",8" tc rgb clr
set label "Speedup\nof Vec.Vec" at -0.7, screen cy-0.05 font ",8" tc rgb clr
set arrow from -0.09, screen cy-0.07 to 0.25, screen cy-0.07 nohead lc rgb clr
set arrow from  0.25, screen cy-0.07 to 0.25, screen cy lc rgb clr

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
