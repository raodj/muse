# Gnuplot script to plot sensitivity analysis results from a given file
# NOTE: ---- This script requires Gnuplot version 4.6 ----

# Run this script as:
# gnuplot -e 'suffix="./tmp/gsa1"' -e 'output="gsa1_anal.pdf"' plot_gsa_anal.gnu
#

# Default directory to find logs (if one is not explicitly set)
if (!exists('output')) {
  output = suffix . "_gsa_anal.pdf"
}
# Setup default error log file if one is not set
csvLog = suffix . "_gsa_anal.csv"

# Set output and plot formats
set terminal pdfcairo enhanced truecolor size 4in,3in font ",12"
set output output

set xtics rotate 270
set yrange[0:0.9]

# Function to get last line from SA error log
genData(x) = system(sprintf("./extract_gsa_for_plot.sh %s %s", suffix, csvLog))
# Extract data from logs into another file for convenient plotting
print genData(1)


# Helper function to extract metric and error for a given parameter index
getData(idx, col) = system(sprintf("grep '^%d,' %s | cut -d',' -f%d", idx, csvLog, col))+0

# Colors for the different parameters plotted by this script
# ColorList="#8dd3c7 #b8860b #928bc1 #fb8072 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"
# getColor(c) = word(ColorList, int(c) + 1)
ColorList="9294791 12092939 9604033 16482418 8434131 16626786 11787881 16078504 5066239 12353725 13429701"
getColor(c) = word(ColorList, int(c) + 1)+0

# Get the number of variables using lines in the output csv file.
MaxVars = system(sprintf("wc -l %s | cut -d' ' -f1", csvLog)) - 2

# Draw the 95% CI bars to see overlaps between metrics
do for [idx=0:MaxVars] {
    metric = getData(idx, 4)
    ci = getData(idx, 5)
    set object rect from graph 0, first (metric+ci) to graph 1, first (metric-ci) fc rgb getColor(idx) fillstyle transparent solid 0.15 noborder back
}

# Set box properties
set boxwidth 0.5
set style fill transparent solid 0.75 border lc rgb '#aaaaaa'
# set style histogram errorbars linewidth 2
set bars front

# Plot labels for the F-metric and 95% CI for the different parameter.
# The labels are plotted separately to make them look pretty
do for[idx=0:MaxVars] {
    # Extract f-round and CI-round columns from the row for the given
    # variable index
    fVal = system(sprintf("grep '^%d' %s | cut -d',' -f6", idx, csvLog)) + 0
    ci   = system(sprintf("grep '^%d' %s | cut -d',' -f7", idx, csvLog)) + 0
    labStr= sprintf("%0.3f\n({/Symbol \\261 %0.2f)", fVal, ci)
    # Set label at correct coordinate
    if (fVal < 0.5) {
        set label labStr center at idx, fVal + 0.11 font " Bold, 8"
    } else {
        set label labStr center at idx, fVal + 0.08 font " Bold, 8"
    }
}

# Plot the data generated from the logs
set datafile separator ","
plot csvLog using 1:4:(getColor($1)):xtic(3) with boxes lc rgb variable  notitle,\
     csvLog using 1:4:5 with yerrorbars lt 7 ps 0.1 lc rgb '#333333' lw 4 notitle

# End of script
