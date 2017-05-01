# A simple R script to run Kolmogorov-Smirinov test on two columns
# in a given CSV file.  The script is meant to be run in the
# following manner:
#
# $ Rscript ks_test.R full_gsa.csv 12 13
#

# options(echo=TRUE)
args <- commandArgs(trailingOnly = TRUE)
# print(args)

# Load specified column from the first CSV file
col1 = as.integer(args[2])
col2 = as.integer(args[3])
x <- read.csv(args[1], header=FALSE)[[col1]]
y <- read.csv(args[1], header=FALSE)[[col2]]

# Run Kolmogorov-Siirinov test
ks.test(x, y)

# End of script
