#!/bin/bash

# Create symbolic link to PHOLD
ln -s $HOME/research/muse/examples/PHOLDSimulation/phold .

# Convenience script to submit PBS jobs to run callgrind with
# different granularity values.

for gran in `seq 21 50`
do
	name="gran_${gran}"
	qsub -j oe -N $name -l"nodes=1:ppn=1" -l"walltime=2:00:00" -v "GRANULARITY=$gran" profPhold.sh
done

# End of script
