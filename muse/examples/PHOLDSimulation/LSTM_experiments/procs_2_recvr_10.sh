#!/bin/bash
#PBS -N procs_2_recvr_10
#PBS -l nodes=1:ppn=12
#PBS -j oe
cd $PBS_O_WORKDIR
jobID=`echo ${PBS_JOBID} | cut -d"." -f1`
outdir='procs_2_recvr_10'
mkdir -p ${outdir}





ST_REP=1
if [ -f ${outdir}/sim_out_1.txt ]; then
  cd ${outdir}
  ST_REP=`ls -1 sim_out_*.txt | tr "_" "." | cut -d"." -f3 | sort -nr | head -1`
  cd -
fi
echo ST_REP = ${ST_REP}
for REP in `seq ${ST_REP} 10`
do
    /usr/bin/time -p -v perf stat -e cpu-clock,task-clock,cycles,instructions,cache-references,cache-misses mpiexec -n 2 ./phold --rows 5 --cols 5 --eventsPerAgent 20 --selfEvents 0 --delay 10 --simEndTime 500 --adapt-time-window  --recvr-range 10 > ${outdir}/sim_out_${REP}.txt 2> ${outdir}/stats_out_${REP}.txt
    cat "${outdir}/stats_out_${REP}.txt" >> "${outdir}/sim_out_${REP}.txt"
done
