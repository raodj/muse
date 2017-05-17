# A GNU plot script to plot charts from Generalized Sensitivity
# Analysis (GSA) results from GA outputs.  Note that this script
# requires GnuPlot version 4.6 or newer.  This script is run as:
#
# $ gnuplot -e plot_delay_chart.gnu


# Setup output file based on input file by chaning extension
# set terminal pdfcairo enhanced color size 3.5in,2.5in font ", 12"
set terminal pdfcairo enhanced color size 3.5in,1.5in font ", 10"
set output "exp_delay_chart.pdf"

# Setup some general properties for all the charts
set grid lc rgb "#cccccc"
set key right top maxrows 6
set xtics
set ytics

set xlabel "Event time increment" font " Bold"
set ylabel "Probability" font " Bold" offset 1, 0

# Colors for the different parameters plotted by this script
ColorList="#8dd3c7 #b8860b #928bc1 #fb8072 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5 #aaaaaa"
getColor(c) = word(ColorList, int(c))

getData(lambda) = sprintf("< ./phold --delay %d --print-hist | grep '^[0-9]' | tr -d '()' | cut -d' ' -f1,2", lambda)
getTitle(lambda) = sprintf("{/Symbol l}=%d", lambda)

# Plot the curve for different lambda values
plot for [lambda=1:12] getData(lambda) using 1:($2/10000) with lines lw 2 lc rgb getColor(lambda) title getTitle(lambda)

# End of script
