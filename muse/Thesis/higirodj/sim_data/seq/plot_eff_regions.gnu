# Gnuplot script to generate a 2-D chart that shows approximate
# regions in the solution space where key data structures were found
# to be the most efficient.

# Set properties for the chart.
set terminal pdfcairo enhanced color size 1.75in,1.75in font ", 10"
set output "eff_regions.pdf"
unset key

# Variables to make script easy to rescale
minX = 800
maxX = 101000

set logscale x
set xrange[minX:maxX]
set xtics("10^{3}" 1000, "10^{4}" 10000, "10^{5}" 100000)
set xlabel "#LPs in model" font " Bold"
set yrange[1:25]
set ylabel "Events/LP" font " Bold" offset 1, 0

ColorList="#8dd3c7 #b8860b #928bc1 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"

heapClr    = word(ColorList, 8);
_3tHeapClr = word(ColorList, 2);
ladderClr  = word(ColorList, 6);

# Draw box for 3tHeap
set obj rect from minX, 5 to maxX, 20 fs transparent solid 0.6 noborder fc rgb _3tHeapClr
set obj rect from minX, 5 to maxX, 20 fs empty border rgb _3tHeapClr

# Draw box for ladder queue region.
set obj rect from minX, 1 to maxX, 6 fs transparent solid 0.3 noborder fc rgb ladderClr
set obj rect from minX, 1 to maxX, 6 fs empty border rgb ladderClr
# Draw box for heap color
set obj rect from minX, 1 to 1000,  2  fs transparent solid 0.6 noborder fc rgb heapClr
set obj rect from minX, 1 to 1000,  2  fs empty border rgb heapClr
set obj rect from maxX-20000, 1 to maxX, 2  fs transparent solid 0.6 noborder fc rgb heapClr
set obj rect from maxX-20000, 1 to maxX, 2  fs empty border rgb heapClr

# Place some labels for key
set label "3tHeap:" at graph 0.05, 0.95
set obj rect at graph 0.445, 0.95 size graph 0.075, 0.075 fs transparent solid 0.6 noborder fc rgb _3tHeapClr
set obj rect at graph 0.445, 0.95 size graph 0.075, 0.075 fs empty border rgb _3tHeapClr

set label "heap:" at graph 0.57, 0.95
set obj rect at graph 0.85, 0.95 size graph 0.075, 0.075 fs transparent solid 0.6 noborder fc rgb heapClr
set obj rect at graph 0.85, 0.95 size graph 0.075, 0.075 fs empty border rgb heapClr

set label "ladderQ:" at graph 0.05, 0.85
set obj rect at graph 0.445, 0.85 size graph 0.075, 0.075 fs transparent solid 0.6 noborder fc rgb ladderClr
set obj rect at graph 0.445, 0.85 size graph 0.075, 0.075 fs empty border rgb ladderClr

set border front linewidth 0.5

plot 0 

# End of script
