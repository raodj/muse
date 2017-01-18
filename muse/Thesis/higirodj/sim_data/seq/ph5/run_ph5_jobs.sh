# These were the commands run on Red Hawk head node to collect data
# for ph5 model.

./generate_ladderq_stats.sh --simExec ./phold --procs "1" --outfile ph5_ladderQ_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue ladderQ --simEndTime 100' > ph5_ladderQ_runtime_log.txt 2>&1 &

./generate_ladderq_stats.sh --simExec ./phold --procs "1" --outfile ph5_2tLadderQ_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue 2tLadderQ --2t-ladderQ-t2k 1 --simEndTime 100' > ph5_2tLadderQ_runtime_log.txt 2>&1 &

./generate_3tHeap_stats.sh --simExec ./phold --procs "1" --outfile ph5_3tHeap_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue 3tHeap --simEndTime 100' > ph5_3tHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph5_2tHeap_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue 2tHeap --simEndTime 100' > ph5_2tHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph5_fibHeap_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue fibHeap --simEndTime 100' > ph5_fibHeap_runtime_log.txt 2>&1 &

./generate_any_queue_stats.sh --simExec ./phold --procs "1" --outfile ph5_heap_runtime_stats.csv --params '--rows 1000 --cols 100 --scheduler-queue heap --simEndTime 100' > ph5_heap_runtime_log.txt 2>&1 &

