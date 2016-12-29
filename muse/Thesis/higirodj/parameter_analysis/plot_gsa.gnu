# A GNU plot script to plot charts from Generalized Sensitivity
# Analysis (GSA) results from GA outputs.  Note that this script
# requires GnuPlot version 4.6 or newer.  This script is run as:
#
# $ gnuplot -e 'csv="gsa1"' plot_gsa.gnu

# NOTE: THE ORDER OF COLUMN TITLES BELOW SHOULD MATCH ORDER IN CSV!!!
# NOTE: Last 2 columns must be timing for ladderQ, heap2tQ!!!
VarNames = "GVTPeriod Imbalance Lambda SelfEvents% SimEndTime Rows EventsPerAgent Granularity Cols"

# Ensure you add trailing '/' to dir
if (!exists('dir')) {
   dir = "./"
}
dataFile = dir . csv . ".csv"
outFile = dir. csv . ".pdf"

# Setup output file based on input file by chaning extension
set terminal pdfcairo enhanced color size 6.5in,6.5in font ", 12"
set output outFile

# Set separator to be comma
set datafile separator ","

# Setup some general properties for all the charts
set grid lc rgb "#cccccc"
# set key right center maxcols 1
set xtics
set ytics

# Convenience recursive function to round to nearest 10th
round(x) = x - floor(x) < 0.5 ? floor(x) : ceil(x)
round10(x, n) = round(x*10**n)*10.0**(-n)
round_nearest(x, n) = round10(x, n) > 0 ? round10(x, n) : round10(x, n+1)

round_tics(x) = system(sprintf("./round_tics %f", x)) + 0

# Setup multiplot with/without title
if (exists("title")) {
    set multiplot layout 3, 3 title title
} else {
    set multiplot layout 3, 3
}
set tmargin 1
set rmargin 1
set lmargin 4

# Setup MAE scale
# maeScale = 1.12

# Set the MAE threshold from CSV file
# maeThresh = system(sprintf("tail -1 %s | cut -d',' -f6", odeFile)) * maeScale
# maeThresh = (maeThresh < 1) ? 1 : maeThresh
# maeThresh = (maeThresh < 2) ? 2 : maeThresh

# print "maeThresh set to: ", maeThresh

# Convenience function to extract min and max x-values from a data file.
getMinX(file) = system("grep -v '^#' ".file." | cut -d',' -f1 | head -1")
getMaxX(file) = system("grep -v '^#' ".file." | cut -d',' -f1 | tail -1")
getMaxF(file) = system("grep '^#' ".file."| tail -1")
hasData(file) = system("test -e " . file . "; echo $?")
toTitle(word) = system(sprintf("echo '%s' | sed 's/[A-Z]/ &/g'", word))

# Helper function to check if a given file exists
file_exists(file) = system("[ -f '".file."' ] && echo '1' || echo '0'") + 0

# Colors for the different parameters plotted by this script
# ColorList="#8dd3c7 #b8860b #928bc1 #fb8072 #80b1d3 #fdb462 #b3de69 #f556a8 #4d4dff #bc80bd #ccebc5"
# getColor(c) = word(ColorList, int(c) + 1)
ColorList="9294791 12092939 9604033 16482418 8434131 16626786 11787881 16078504 5066239 12353725 13429701"
getColor(c) = word(ColorList, int(c) + 1) + 0

# Draw sub-plots for each parameter
MaxVars = words(VarNames)
do for [i=1:MaxVars] {
    # Generate GSA results using c++ program
    outFile = dir . csv . "_" . word(VarNames, i) . "_gsa.csv"
    # if (!file_exists(outFile)) {    
        print "Generating GSA data in ", outFile
        print sprintf("./gsa %s %d -2 -1 > %s", dataFile, i, outFile)
        system(sprintf("./gsa %s %d -2 -1 > %s", dataFile, i, outFile))
    # }

    # Check to ensure we had a valid run and there is some data to process
    lineCount = system(sprintf("wc -l %s", outFile))
    if (lineCount > 3) {
        # NOTE: This must be done before setting y range!
        # For y2-range we need to scale based on max number of trials
        set yrange[*:*]
        stats outFile using 9 name 'entries' nooutput
        maxY2 = entries_max * 2.5
        set y2range [0:maxY2]
    
        # Set x-range based on data we have.
        minX = getMinX(outFile)
        maxX = getMaxX(outFile)
        
        set yrange [0:1]
        set xrange [minX:maxX]
        tics = (maxX - minX) / 5.0
        print "maxX=", maxX, ", minX=", minX, ", tics=", tics
        tics = (floor(tics) > 0) ? floor(tics) : tics
        approxTics = round_tics(tics)
        print "xtics set to:", tics, " approx: ", approxTics
        set xtics approxTics
        
        # Set title outside chart
        set title toTitle(word(VarNames, i)) offset 0,-1 font ",12" tc rgb '#0072bd'
        
        # Draw arrows at the max F value (use last line in CSV)
        arrowVals = getMaxF(outFile)
        aroX  = word(arrowVals, 2) + 0
        aroY1 = word(arrowVals, 3) + 0
        aroY2 = word(arrowVals, 4) + 0
        maxFVal = word(arrowVals, 5) + 0
        ci    = word(arrowVals, 6) + 0
        maxF = sprintf("%.3f", maxFVal)
        minY = (aroY1 < aroY2) ? aroY1 : aroY2
        maxY = (aroY1 < aroY2) ? aroY2 : aroY1
        midY = (maxY + minY) / 2

        # Draw dotted line, arrows, and F statistic 
        set arrow 1 from aroX, aroY1 to aroX, aroY2 nohead ls 0 lw 5 lc rgb '#d95319' front
        set arrow 2 from aroX, (minY - 0.05) to aroX, minY lw 2 lc rgb '#d95319' front
        set arrow 3 from aroX, (maxY + 0.05) to aroX, maxY lw 2 lc rgb '#d95319' front
        # Draw horizontal 95% CI lines for the F measures
        set arrow 4 from minX, (maxFVal - ci) to maxX, (maxFVal - ci) nohead lw 0.1 lc rgb getColor(i) front
        set arrow 5 from minX, (maxFVal + ci) to maxX, (maxFVal + ci) nohead lw 0.1 lc rgb getColor(i) front
        set object 6 rect from minX, (maxFVal - ci) to maxX, (maxFVal + ci) fc rgb getColor(i) fillstyle transparent solid 0.15 noborder front
        # Place label above or below depending on where it fits better
        labY = (minY > 0.35) ? (minY - 0.125) : (maxY + 0.125)
        labY = (labY > 0.95) ? 0.95 : labY
        # Set label to be to the left and right to ensure readability
        midX = minX + (maxX - minX) / 4
        if (aroX <= midX) {
            set label 4 maxF at aroX, labY left front tc rgb '#d95319' font "Arial-Bold"
        } else {
            set label 4 maxF at aroX, labY right front tc rgb '#d95319' font "Arial-Bold"
        }
        # Plot the data.
        plot outFile using 1:9 axes x1y2 with impulses lw 0.25 lc rgb '#ffdf00' notitle,\
             outFile using 1:4 with lines lw 4 lc rgb "#628b27" notitle, \
             outFile using 1:7 with lines lw 4 lc rgb "#7e2f8e" notitle
    } else {
        print "Insufficient data in ", outFile         
    }
}

# Unset multiplot to actually draw the chart into a pdf
unset multiplot

# End of script.
