The data in this directory was collected in the following manner:

1. The LadderQueue.h file was edited to set the data structure for bucket:
       // using Bucket = ListBucket;
       using Bucket = VectorBucket;

2. Next the LadderQueue.h header was edited to select the
   implementation for Bottom by uncommenting one of the following
   lines:

       Bottom bottom;
	   // HeapBottom bottom;
	   // MultiSetBottom bottom;

3. Next the kernel and PHOLD was compiled and copied to different
   executables in this directory.

4. A series of simulations were run by suitably modifying the name of
   the executable, output file, and log file:

   ./generate_any_queue_stats.sh --reps 3 --procs "1" --outfile ph3_lq_vec_list_timings.csv --params '--rows 100 --cols 10 --scheduler-queue ladderQ --simEndTime 5000' > ph3_lq_vec_list_log.txt 2>&1 &

5. The above process is repeated 5 more times with different
   combinations for 'Bucket' and 'bottom'

