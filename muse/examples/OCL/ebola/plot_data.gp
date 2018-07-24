# Gnuplot script to visually compare results from MUSE and the data
# published by Rivers et. al.

# Set output to file formats
set terminal pdfcairo enhanced color size 6.5in,4in font ",12"
set output "vv_liberia.pdf"
set datafile separator ","

set multiplot

set key at 75, 25000
pop = 470000
blue = '#0072bd'

set yrange[0:200000]
set xrange[0:280]

set ylabel 'Cumulative Infections' tc rgb blue font " Bold" offset 1, 0
set xlabel 'Time (Days)' tc rgb blue font " Bold" offset 0, 0.5

plot 'liberia_ode.csv' using 2:(pop - $3) with lines lw 5 lc 0 title "MUSE-HC: ODE",\
     'liberia_ssa.csv' using (174 + $2):(pop - $3) with lines lw 5 lc 3 title "MUSE-HC: SSA"

# Plot chart from Rivers et al inside this chart at the top-left corner

set origin -0.02, 0.26
set size 1, 0.65
set size ratio -1
unset xtics
unset ytics
set xrange [0:857]
set yrange [0:503]
unset xlabel
unset ylabel
set noborder
set lmargin 0
set tmargin 0

plot 'rivers_liberia.jpg' binary filetype=jpg with rgbimage

# It's important to close the multiplot environment!
unset multiplot

# End of script
