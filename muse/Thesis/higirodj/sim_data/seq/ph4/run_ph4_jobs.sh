# These were the commands run on Red Hawk head node to collect data
# for ph4 model (on Sat Jan 14)

ln -s ../../generate_ladderq_stats.sh .
ln -s ../../generate_3tHeap_stats.sh .
ln -s ../../generate_any_queue_stats.sh .
ln -s ../../generate_stats.sh .
ln -s ../../stats_funcs.sh .
ln -s ~/research/muse/examples/PHOLDSimulation/phold .

./generate_ladderq_stats.sh --simExec ./phold --procs "1" --outfile ph4_ladderQ_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue ladderQ --simEndTime 500' > ph4_ladderQ_runtime_log.txt 2>&1 &

./generate_ladderq_stats.sh --simExec ./phold --procs "1" --outfile ph4_2tLadderQ_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue 2tLadderQ --2t-ladderQ-t2k 1 --simEndTime 500' > ph4_2tLadderQ_runtime_log.txt 2>&1 &

./generate_3tHeap_stats.sh --simExec ./phold --procs "1" --outfile ph4_3tHeap_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue 3tHeap --simEndTime 500' > ph4_3tHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph4_2tHeap_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue 2tHeap --simEndTime 500' > ph4_2tHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph4_fibHeap_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue fibHeap --simEndTime 500' > ph4_fibHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph4_heap_runtime_stats.csv --params '--rows 100 --cols 100 --scheduler-queue heap --simEndTime 500' > ph4_heap_runtime_log.txt 2>&1 &
