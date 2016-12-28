This directory contains files, scripts, and data generated via
generalized sensitivity analysis (GSA) of the different parameters in
PHOLD.  Specificaly, the GSA aims to identify the significance of the
parmaeters in PHOLD that have the most influence on runtime of PHOLD
with two different scheduler queues, namely 'ladderQ' vs 'heap2tQ'.

The data for GSA is collected and plotted using the following
procedure:

1. First clean-up any existing .csv files (if needed)

2. Generate raw GSA data by running many replications of PHOLD using
   different combination of parameters using ./submit_gsa_jobs.sh

3. Wait for all the jobs to finish. Check to ensure the gsa_?.csv
   files look good.

4. Now process the data and generate charts using ./plot_gsa_data.sh

Look at the data in the chart. Parameters with high f-value are
signficiant.  Conversely, parameters with low f-value (say < 0.2) are
not significant/influential to distinguish the performance of the two
queues.


