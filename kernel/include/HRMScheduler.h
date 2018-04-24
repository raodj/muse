#ifndef MUSE_HRM_SCHEDULER_H
#define MUSE_HRM_SCHEDULER_H

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

#include "Scheduler.h"
#include "ResChannel.h"
#include "ArgParser.h"

BEGIN_NAMESPACE(muse);

/** Scheduler that is aware of Hierarchical Resource Management (HRM).

    <p>This scheduler extends MUSE's default Fibonacci Heap scheduler
    to provide interaction with Hierarchical Resource Management
    (HRM), as reported in the following paper:</p>

    <p><i>"Hierarchical resource management for enhancing performance
    of large-scale simulations on data centers"</i>, by Zengxiang Li,
    Xiaorong Li, Long Wang, and Wentong Cai, in Proceedings of the 2nd
    ACM SIGSIM/PADS conference on Principles of Advanced Discrete
    Simulation (SIGSIM-PADS'14), ISBN: 978-1-4503-2794-7, Pages
    187-196, May 18-21, Denver, Colorado, USA.  DOI:
    10.1145/2601381.2601390.</p>

    <h2>Concepts and Terminology:</h2>

    <ul>

    <li>\b Epoch: In this scheduler an "Epoch" is a virtual time that
    is reached when all <i>local</i> events currently scheduled at
    that time have been processed (they could be rolledback but that
    does not matter).</li>
    
    </ul>
*/
class HRMScheduler : public Scheduler {
public:
    /** \brief Default Constructor
        
        Does not have a specific task to perform and is merely present
        as a place holder for future extensions.
    */
    HRMScheduler();

    /** \brief Destructor
        
        Does nothing, as the constructor does nothing.
    */
    virtual ~HRMScheduler();

    /** \brief Track time advancement to detect a new Epoch.

        This method overrides the default method in the base class to
        detect each Epoch occurrence during simulation.  This
        information is collated into a queue to be reported to the HRM
        domain manager upon request (by the domain manager).  The base
        class performs the actual task of scheduling events to variou
        agents.  This method adds the functionalit for Epoch detection
        an epochList management.

        \note This method is only used by the Simulation kernel. Users
        of MUSE API should not touch this function.
        
        \return This method returns \c true if the next agent had
        events to process.  If no events were available then this
        method returns \c false.
    */
    virtual AgentID processNextAgentEvents();

    /** The garbage collection method for scheduler.
        
        This method is called when it is safe to garbage collect for a
	given Global Virtual Time (GVT).  This method communicates the
	information to the resource channel so that it can safely
	garbage collect.

        \param gvt, this is the GVT time that is calculated by GVTManager.
    */
    virtual void garbageCollect(const Time& gvt);

    /** Method invoked just before the core simulation starts to run.

        This method is invoked after initialization and all the agents
        have been registered and they are about to be initialized
        (from Simulation::start() method).  This method calls
        ResChannel::informStart() method to inform the HRM domain
        controller that the simulation is starting.

        \parma[in] startTime The logical starting time of the
        simulation.  This parameter is not used by this method.
    */
    virtual void start(const Time& startTime);

    /** Method invoked just after the core simulation is complete.

        This method is invoked after all the agents have been
        finalized and the simulation has successfully completed.  This
        method calls ResChannel::informTerminate() to indicate that
        the simulation is complete.
    */
    virtual void stop() { resChannel.informTerminate(); }

protected:
    /** \brief Override base class implementation to manage Epoch
        List as necessary.

        This method passess control to the base class to first process
        rollbacks.  If a rollback occurs, then this removes
        corresponding Epoch entries from the epoch list.

        \param[in] e The event to check.  This parameter cannot be
        NULL.
        
        \param[in,out] agent The agent to check against.  This agent
        must be the agent to which the event is destined.  This
        pointer cannot be NULL.

        \return This method returns \c true if a rollback occured.  If
        the event did not cause a rollback (\i i.e., the event was not
        a \i stragger) then this method returns \c false.
    */
    bool checkAndHandleRollback(const Event* event, Agent* agent);

    /** Convenience method to obtain current real/wall-clock time in
        milliseconds.

        This method uses gettimeofday Linux call to obtain the current
        system time in microseconds and converts it to milliseconds.

        \return The current system time in milliseconds.
    */
    unsigned long currentTimeMillis() const;

    /** \brief Complete initialization of the Scheduler.

        Once the scheduler instance is created by
        Simulation::initialize() method, it must be full initialized.
        This includes processing any command-line arguments to further
        configure the scheduler.  This method overrides the base class
        method to process the following command-line arguments:

        <ul>
        <li>\b --domain-ctrl-list : A list of space separated entries 
        </ul>

        \param[in] rank The MPI rank of the process associated with
        this scheduler.

        \parma[in] numProcesses The total number of parallel processes
        in the simulation.
        
	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.
    */
    virtual void initialize(int rank, int numProcesses, int& argc, char* argv[])
        throw (std::exception);

    /** Convenience method to obtain host and IP of domain controller
        for this MPI rank.

        <p>This method is a refactored utility method currently used
        only once from the initialize method.  This mehtod is used to
        identify information (host and port number) of the domain
        controller associated with this parallel process.  This method
        identifies the host controller from either: (a) the explicit
        list of controllers specified as command-line arguments, \i or
        (b) a list of controllers specified in a given file (path to
        file is specified as command-line argument).  The two options
        are detailed further below:</p>

        <ul>

        <li>\b \c --hrm-controller-list : This method gives preference
        to the command-line arguments specified in the from
        <tt>--hrm-controller-list node1:3000 node2:4000 node7:3000
        node1:5000</tt>.  In this list the HRM controller for rank 0
        is on "node1:3000", for rank 1 is "node2:4000", and so forth.</li>

        <li>\b \c --hrm-controller-file : If an explicit list of
        controllers is not specified then this method loads domain
        controller information from a given file. In this file, each
        line is assumed to represent a domain controller for the
        corresponding MPI rank.  Each domain entry is in the form
        hostname:port_number as shown below:

        <p>Here is an example of a domain controller file for 4
        domains corresponding to MPI rank 0 through 3, in
        \c host:port_number format :</p>

        <pre>
        node1:3000
        node2:4000
        node7:3000
        192.168.0.121:5000
        </pre>

        In the above file the controller for rank 0 is on
        "node1:3000", for rank 1 is "node2:4000", and so forth.
        </li>
        </ul>
        
        \param[in] filePath The path to the domain controller file
        from where the data is to be loaded.  If the file cound not be
        read or if the file does not have sufficient entries for all
        the ranks (there can be more entries but not fewer than the
        number of parallel processes in the simulation), then this
        method reports an error and aborts.

        \param[in] rank The rank of the process for which the domain
        controller information is to be returned by this method.

        \return The domain controller information for the given rank.
        The returned string is of the form host:port_number.
    */
    std::string getDomainControllerInfo(ArgParser::StringList& cmdLineList,
                                        const std::string& filePath, int rank) const;
    
private:
    /** Instance variable to track the previous Epoch time to
        detect occurrence of a new Epoch.

        In this scheduler Epoch detection is performed by tracking the
        previous simulation time and comparing it with current
        simulation time.  If the previous time and current time are
        different then this a new Epoch is detected.  This value is
        suitably updated whenever a rollback occurrs.
    */
    Time prevEpochVTime;

    /** Instance variable to track the real/wall-clock time in milliseconds.

        This instance variable tracks the wall clock time of the
        previous Epoch.  This variable is initialized to zero and
        reset to zero after each rollback (where prevEpochTime is
        updated).  The value is set in the processNextAgentEvents()
        method and used to measure the real time elapsed for advancing
        through a set of Epochs.
    */
    unsigned long epochClock;

    /** Configuration variable to define real time advancement
        threshold value.

        This value specifies the avancement required in fqrClock to
        detect when a new Epoch is to be created.  Generally, an epoch
        is created for each advancement in virtual time.  However, due
        to scheduling issue, the measured performance may be
        inaccurate if the epoch is too short.  Hence, epochs are
        created only if the interval of requested time and wall-clock
        are greater than this threshold value.  The default value is
        set to 30 milliseconds.
    */
    unsigned long epochClockThreshold;

    /** Configuration variable to define virtual time advancement
        threshold value.

        This value specifies the avancement required in virtual time
        to detect when a new Epoch is to be created.  Generally, an
        epoch is created for each advancement in virtual time.
        However, due to scheduling issue, the measured performance may
        be inaccurate if the epoch is too short.  Hence, epochs are
        created only if the interval of requested time and wall-clock
        are greater than this threshold value.  The default value is
        set to 10 time units of virtual time (note that virtual time
        does not have an unit).
    */
    Time epochVTimeThreshold;

    /** The resource channel via wich communication is performed with
        the domain controller.

        This class manages the Epoch list and communicates with the
        domain controller.
    */
    ResChannel resChannel;
};

END_NAMESPACE(muse);

#endif
