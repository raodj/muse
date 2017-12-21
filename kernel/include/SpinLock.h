#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <atomic>

BEGIN_NAMESPACE(muse);

/** A standard spin lock for use in multithreaded applications.

    This is a standard spin lock implementation adatped from
    Boost. The spin lock is a substitute for std::mutex and can
    provide lower latency access to critical sections.  The spin lock
    can provide improved performance only when the following 2 cases are met:

    <ol>

    <li>The critical section is very short with a few instructions.</li>

    <li>The demand for accessing the critical section from many
    threads is high.</li>

    </ol>

    Note that the spin lock is more CPU intensive as it keeps
    spinning/looping and does not suspend/yield threads.
*/
class SpinLock {
private:
    /** Enumerations for a couple of states to keep th code readable. */
    typedef enum {Locked, Unlocked} LockState;

    /** The key atomic variable used to determine the state of the
        lock to see if it is locked/unlocked. */
    mutable std::atomic<LockState> state;
public:
    /** The only constructor for the spin lock.  The lock is
        initialized ot the 'unlocked' state.
    */
    SpinLock() : state(Unlocked) {}

    /** Method to obtain a lock.

        This method loops indefinitely until the calling thread is
        able to lock the spin lock.

        \note It is important to call unlock after use of the critical
        section.
    */
    inline void lock() const {
        while (state.exchange(Locked, std::memory_order_acquire) == Locked) {
            /* busy-wait */
        }
    }

    /** Mehtod to unlock so other threads can obtain the lock.

     */
    inline void unlock() const {
        state.store(Unlocked, std::memory_order_release);
    }
};

END_NAMESPACE(muse);

#endif
