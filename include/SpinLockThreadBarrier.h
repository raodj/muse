#ifndef SPIN_LOCK_BARRIER_H
#define SPIN_LOCK_BARRIER_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <thread>
#include <atomic>

/** A simple barrier based on spin locks for multiple thread.

    This class provides a simple reusable barrier for multiple
    threads.  Internally, threads wait for all threads to hit the
    barrier using a spin lock.  A single (typically static) instance
    of this class must be used by all threads to hit the barrier.
*/
class SpinLockThreadBarrier {
public:
    /** Create a barrier object to be used by a given number of
        threads.

        \param[in] count The number of threads that will be
        sharing/using this barrier.
    */
    explicit SpinLockThreadBarrier(int count = 0) :
        pendingThrCount(count), generation(0), threadCount(count)  {
        // Nothing else to be done in the constructor
    }

    /** Set the number of threads that will be sharing/using this
        barrier.

        This is a convenience method that can be used to override the
        number of threads that will be using this barrier.  This
        method must be used well before the threads start using the
        barrier.  Typically it is best to set this value before the
        threads are even spun-up.

        \param[in] count The number of threads that will be using this
        barrier to coordinate their operations.
     */
    void setThreadCount(int count) {
        pendingThrCount = count;        
        threadCount     = count;
    }

    /** Wait for all the threads to hit the barrier.

        Once this method is invoked it returns only after the
        specified numebr of threads have also hit the barrier by
        calling this method.  This method internally spins (while
        yielding the thread) until specified number of threads have
        hit the barrier.  Once all the threads have hit the barrier,
        the internal counters are reset so that the same barrier can
        be reused for the next round of coordination.
    */
    void wait()   {
        int gen = generation.load();
        // Atomically decrement the pending thread count.
        if (--pendingThrCount == 0) {
            // Pending thread count was zero. Barrier met. Reset
            // barrier for next round by incrementing the generation.
            pendingThrCount = threadCount;
            if (!generation.compare_exchange_weak(gen, gen + 1)) {
                abort();
            }
        } else {
            // Spin until all threads hit the barrier and pendingThrCount
            // drops to zero, and the generation is inremented.
            while (gen == generation) {
                std::this_thread::yield();
            }
        }
    }

    /** Obtain the current generation of the barrier being used.

        This is just a convenience method to determine the internal
        generation of the barrier.

        \return The number of times this barrier has been successfully
        completed so far.
    */
    int getGeneration() const { return generation; }
    
protected:
    /** Ensure that the barrier objects are not copyable. */
    SpinLockThreadBarrier(const SpinLockThreadBarrier&) = delete;

    /** Ensure that this type of object is not assignable */
    SpinLockThreadBarrier& operator=(const SpinLockThreadBarrier&) = delete;
    
private:
    /** Instance variable to track the number of threads that have not
        yet hit the barrier and decremented this atomic/shared
        variable.
    */
    std::atomic<int> pendingThrCount;

    /** This counter is used to track the next iteration of the
        barrier to ensure consistent reuse of the barrier object.
        After all threads hit the barrier, this value is incremented
        and the pendingThrCount is reset.
    */
    std::atomic<int> generation;

    /** The number of threads that are sharing this spin lock.  This
        value must be set before the threads start using the
        barrier.
    */
    int threadCount;
};

#endif
