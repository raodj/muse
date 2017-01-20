# Gnuplot script to plot sensitivity analysis results from a given file
# NOTE: ---- This script requires Gnuplot version 4.6 ----

# Run this script as:
# gnuplot plot_granularity.gnu

# Set output and plot formats
# set terminal pdfcairo enhanced truecolor size 4in,3in font ",12"
set terminal pdfcairo enhanced truecolor size 4in,1.5in font ",10"
set output "gran_info.pdf"

set xrange[-1:51]
set yrange[0:100]
set grid lc rgb "#cccccc"
set key left

blue="#0072bd"
ltBlue="#4dbeee"
red="#a2142f"

set xlabel "Granularity parameter value" font " Bold"
set ytics  format "%2.0f%%" tc rgb blue nomirror
set ylabel "Percent of simulation\ntime spent in model" tc rgb blue font " Bold"
set y2label "Instructions per event" tc rgb red font " Bold"
set y2tics offset -1.5, 0 rotate by -45 tc rgb red

# Set box properties
set boxwidth 0.75
set style fill transparent solid 0.5 border lc rgb ltBlue
set bars back

plot "gran_info.csv" using 1:2 with boxes lc rgb ltBlue title "%Run time", \
     "gran_info.csv" using 1:3 axis x1y2 with lines lw 3 lc rgb red title "Instr./event"

# End of script

