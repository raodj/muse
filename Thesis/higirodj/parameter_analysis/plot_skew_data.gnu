# A GNU plot script to plot charts from Generalized Sensitivity
# Analysis (GSA) results from GA outputs.  Note that this script
# requires GnuPlot version 4.6 or newer.  This script is run as:
#
# $ g++ -g -Wall -std=c++11 skew_data.cpp -o skew_data
# $ gnuplot plot_skew_data.gnu

# NOTE: You must compile skew_data.cpp prior to running this script

# Setup output file based on input file by chaning extension
set terminal pdfcairo enhanced color size 3.5in,3.5in font ", 12"
set output "skew_data.pdf"

# Set properties of the data and chart
set datafile separator ","
set style data histogram
set style histogram rowstacked
set style fill solid border -1
set boxwidth 0.75

# Setup some general properties for all the charts
set grid lc rgb "#cccccc" front
set key top left  maxrows 2
set yrange [0:115]
set xtics
set xlabel "Imbalance (percentage)" font " Bold"
set ytics
set ylabel "LPs per partition (Part.)" font " Bold" offset 1,0

plot '< ./skew_data 100 4' using 2:xtic(1) lc rgb '#f0bc42' title "Part. #1",\
     '' using 3:xtic(1) lc rgb '#b3cde3' title "Part. #2", \
     '' using 4:xtic(1) lc rgb '#ccebc5' title "Part. #3", \
     '' using 5:xtic(1) lc rgb '#decbe4' title "Part. #4"

# End of script
