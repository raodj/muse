#ifndef GVT_MANAGER_H
#define	GVT_MANAGER_H

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

#include "DataTypes.h"
#include "GVTManagerBase.h"

BEGIN_NAMESPACE(muse);

// Forward declarations to keep compiler happy and fast
class Simulation;
class GVTMessage;
class Communicator;
class Event;

/** A class that implements the Mattern GVT algorithm.

    \file This file contains the implementation for computing the
    Global Virtual Time (GVT) using Mattern's algorithm.

    <p>This class contains various methods that implement Mattern's
    Global Virtual Time (GVT) algorithm. The algorithms implemented in
    this class is decribed in the following paper:</p>

    <p>Friedmann Mattern, <i>"Efficient algorithms for distributed
    snapshots and global virtual time approximation"</i>, Journal of
    Parallel and Distributed Computing, 1993, Vol. 18, Pages:
    423--434.</p>
*/
class GVTManager: public GVTManagerBase {
    // Declare muse::Simulation to be a friend so that it can
    // instantiate this GVTManager class for its use.
    friend class Simulation;
    friend class oclSimulation;
public:
    /** Initialize the internal data structures for GVT calculations.

        This method must be used to initialize the internal data
        structures and values used by the GVT manager for computing
        GVT.  Note that the number of physical processes and rank of
        the local process must be known for GVT
        calculation. Therefore, this method is invoked typically after
        the Communicator has been initialized (typically involves
        initializing MPI) and information about the physical size of
        the simulation is known (this information may not be known
        when the simulation is instantiated).

        \note This method must be invoked to initialize the GVT
        manager even before the agents in the process are initialized.
        
        \param[in] startTime The vritual time when the simulation is
        set to start.  This value must be exactly the same on all
        processes associated with the simulation.  This value is used
        to immediately set the GVT value just before the simulation
        commences.

        \param[in] comm The communicator object via which GVT control
        messages are to be dispatched.  The communicator is also used
        to determine information about the processes involved in the
        simulation.
    */
    void initialize(const Time& startTime, Communicator* comm);

    /** Method to update information and send a remote event.

        This method must be used to dispatch an outgoing event to
        another Agent present on a different process.  This method
        performs the following tasks:

        <ol>

        <li>It first updates the color of the event to the currently
        active color indicated by activeColor instance variable.</li>

        <li>It then dispatches the event via the communicator to the
        remote process.</li>

        <li>Next, it increments the vector entry associated with the
        destination process and the current activeColor.  Note that
        this step is a bit different from the algorithm described in
        Mattern's paper because the GVT algorithm is going to be
        repeatedly executed and we need to track both red and white
        messages.</li>

        <li>If the color is red (note that red is not a fixed value
        and flip-flops between 1 and 0 from round to round. However,
        thinking of the color as red helps to follow the GVT algorithm
        as described in the paper) it tracks and updates tMin (the
        smallest time stamp of outgoing messages) to the minimum
        receiveTime value.</li>

        </ol>
        
        \param[in] event The event to be updated and dispatched to an
        remote process.

        \return This method returns \c true if the event was
        dispatched successfully.  On errors this method returns \c
        false.
    */
    bool sendRemoteEvent(Event* event);

    /** Method to inspect an incoming remote event and update vector
	counters.

        This method is invoked from Communicator::receiveEvent()
        method to inspect all \b incoming events from other processes.
        This method performs the following tasks:
	
        <ol>
	
        <li>It decrements the vector counter associated with the
        receiving process for the color associated with the event.<li>
	
        </ol>
    */
    void inspectRemoteEvent(Event* event);
   
    /** Obtain the current estimate of GVT.

        This method can be used to obtain the currently estimated GVT
        value for the simulation.
        
        \return The currently estimated GVT value for this simulation.
    */
    inline Time getGVT() { return gvt; }

    /** Handle incoming GVT-related messages.

	This method is invoked from the Communicator whenever it
	receives a message pertaining to GVT computations.  This
	method performs the following tasks:

	<ol>

	<li>If the message is of type \c GVT_CTRL_MSG it calls the
	handleCtrlMsg() method to process the message.</li>

	<li>If the message is of type \c GVT_EST_MSG and the rank of
	this process is not 0, then it: first sets new GVT estimate
	value via the setGVT method and second it forwards the message
	to the next process in the ring.  Finally it deletes the
	message as this kind (namely GVT_EST_MSG) of message is not
	retained.</li>
	
	</ol>
	
	\param[in,out] message The incoming GVT message to be
	processed by this method.
    */
    void recvGVTMessage(GVTMessage* message);

    /** Method to initate a new round of GVT estimation.

        This method is periodically invoked from the
        Simulator::start() method to initate GVT estimation process if
        necessary.  If a GVT estimation process is already underway
        (as indicated by the \c cycle instance variable) then this
        method performs no further operations and returns immediately.
        Otherwise this method initates a new control message and
        dispatches it to the next process for GVT estimation.

        \note This method has effect only on process with rank 0 which
        is the initiator process.  Calling this method on other
        processes has no effect at all.
    */
    void startGVTestimation();


    /** Helper method to check and forward pending control message.
        
        This is a refactored utility method (that is invoked from
        inspectRemoteEvent and recvGVTMessage) to check if a pending
        wait condition has expired.  This method essentially checks to
        see if the wait condition (as described in Mattern's paper)
        has expired.  If so it either updates GVT (if the process is
        the initator) or forwards the pending control message to the
        next process (using the forwardCtrlMsg method).  If the wait
        has not expired the this method does not perform any other
        operations.
    */
    void checkWaitingCtrlMsg();

    /** Convenience method to override MPI rank with a thread-based
        rank.

        This method is typically used by MultiThreadedSimulation to
        override the MPI-based rank set in initialize method with
        thread-based rank.  This is important because each thread can
        have its own custom GVT manager and consequently the rank
        associated with them needs to be different.  The thread-based
        rank cannot be determined by the GVT manager.  Instead, it
        needs to be set my the Simulation hierarchy that owns the GVT
        manager.

        \note Call to this method is meaningful only after
        GVTManager::initialize method has been invoked on this GVT
        manager.

        \note This method only used with non-memory sharing multi-
        threading. Shared memory multi-threading does not use thread
        based MPI rank.
        
        \param[in] thrRank The thread-based rank to be used instead of
        MPI rank.
    */
    void setThreadedRank(const int thrRank) { rank = thrRank; }
    
protected:
    /** Helper method to set the GVT value.

        This is a helper method that is invoked from
        checkWaitingCtrlMsg (for rank 0) or recvGVTMessage (for other
        ranks).  This method is used to set GVT and trigger garbage
        collection only on processes with non-zero rank.  For rank 0,
        we wait until all GVT acknowledgment messages are received to
        ensure that the simulation loop does not terminate early
        (which may cause some acknowledgements to be
        lost/unprocessed).  In the case rank 0, the GVT is set when
        acknowledgments reach zero.

        \param[in] gvtEst The newly estimated GVT value.
    */
    void setGVT(const Time& gvtEst);
    
    /** The constructor.

        <p>The constructor has been made private to ensure that it is
        not instantiated by any other class other than
        muse::Simulation.  This is to ensure that only the simulation
        can own a GVTManager object.</p>

        The constructor is relatively straightforward and sets all of
        the instance variables to intial (invalid) values. The
        variables are initialized to valid values by the initialize
        method.

        \param[in] sim Pointer to the simulator that logically owns
        this GVTManager. The pointer is used to determine the LGVT
        value.
    */
    GVTManager(muse::Simulation *sim);

    /** The destructor.
	
        The destructor cleans up the dynamic memory allocated to hold
        the vector counters (if any) in this class.
    */
    virtual ~GVTManager();
     
    /** Helper method to update and forward a GVT control message.

        This is a helper method that is used to update and forward a
        control message that is currently present in this GVTManager.
        This method performs the following tasks:

        <ol>

        <li>It first updates the vector counters in the event and
        resets the local vector counters (for white color) to
        zero.</li>

        <li>It then updates the tMin value based on the tMin value
        present in this class.</li>

        <li>It updates the estimated GVT value depending on whether
        this process is the initiator (rank 0) or non-initiator</li>

        <li>It then forwards the control message to the next process
        in the ring.</li>

        <li>Finally it destroys the current control message and sets
        ctrlMsg instance variable to NULL.
        
        </ol>
    */
    void forwardCtrlMsg();

    /** Convenience method to check and update GVT and trigger garbage
        collection.

        This is a refactored method that is used to tirgger garbage
        collection.  It is called from setGVT method for non-zero-rank
        processes.  For rank 0, it is invoked from 
        
        \parma[in] gvtEst The newly estimated GVT value.
     */
    void garbageCollect(const Time& gvtEst);
    
    
private:
    /** The current active color associated with this process.

        This instance variable holds the current active color for this
        process.  The current active color is initialized to white and
        then it is changed to red when the GVT algorithm is initiated.
    */
    char activeColor;

    /** The current value associated with the color white.

        This instance variable holds the current value associated with
        the color white.  This value starts with 0. Each time a GVT
        calculation is completed the actual values associated with the
        colors red and white are flipped in preparation for the next
        cycle of GVT counts.

        \note To minimize instance variable, this class does not
        maintain the value for color red. Instead, the value is
        determined as \c !white.
    */
    char white;

    /** The vector counters associated with white and red events.

        This instance variable maintains the list of vector counters
        for the two colors red and white. The actual list of entries
        are created in the initialize method once the total number of
        processes in the simulation is known.  In the algorithmic
        description in Mattern's paper, this vector is called \c V.
    */
    int* vecCounters[2];
    
    /** Instance variable to maintain minimum timestamp of output
        going events.

        This instance variable corresponds to the \c tmin variable
        used in the algorithmic description in Mattern's paper. This
        instance variable tracks the minimum value of \c receiveTime
        values in all outgoing \i red events dispatched by this
        process.
    */
    Time tMin;

    /** Instance variable to hold any pending GVT control message.

        This instance variable is used to hold a pending GVT control
        message.  This instance variable is initialized to NULL in the
        constructor.  A valid control message is set in the
        recvGVTMessage() method.  The control message is updated and
        forwarded in the inspectRemoteEvent() method.
    */
    GVTMessage* ctrlMsg;

    /** The current cycle of GVT calculations that is underway.

        This instance variable is used to track the current cycle of
        GVT calculations that is currently underway.  A value of zero
        indicates that GVT calculations are currently not pending.
    */
    int cycle;

    /** The number of pending acknowledgements for a GVT update.

	This instance variable is set to the number of processes-1
	each time a GVT cycle is complete and gvt time updates are
	broadcasted (by the process with rank zero) to processes.
	This variable is used to wait until all the processes
	acknowledge receipt of GVT messages.  Waiting for
	acknowledgements is necessary to ensure that the next cycle of
	GVT computations does not commence until all the processes
	have had a chance to receive the broadcast from process 0.
    */
    int pendingAcks;
};

END_NAMESPACE(muse);

#endif
