#ifndef GVT_MESSAGE_H
#define	GVT_MESSAGE_H

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
// Authors:  Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "DataTypes.h"

BEGIN_NAMESPACE(muse);
class GVTMessage;
END_NAMESPACE(muse);

// Forward declaration for insertion operator for Event
extern std::ostream& operator << (std::ostream&, const muse::GVTMessage&);

BEGIN_NAMESPACE(muse);

/** \file GVTMessage.h

    \breif The file declares a GVTMessage class that is used to
    encapsulate information about GVT tokens circulated during
    simulation.

    The file declares a GVTMessage class and associated information.
    Objects of GVTMessage type are used to encapsulate information
    required for calculating the Global Virtual Time(GVT) value during
    simulation.  GVT is needed for automatic garbage collection and to
    terminate the simulation.
*/

/** \brief A class to encapsulate information associated with GVT
    messages.

    This class essentially serves to encapsulate information about
    Global Virtual Time (GVT) messages that are circulated between
    processes to estimate GVT.  GVT is necessary for grabage
    collection and for termination of the simulation.

    This class represents the following two types of GVT messages that
    are currently used as a part of GVT calculations:

    <ul>

    <a id="gvt_ctrl_msg">
    <li> \c GVT_CTRL_MSG: This type of message (indicated by the \c
    kind member in this class) is used for circulating gvt control
    messages (as described in Mattern's paper) that contain the
    following information: <\c m_clock, \c m_send, \c count>, where:

    <ul>
    
    <li> \c m_clock indicates the cumulative minimum Local GVT value
    on all the processes that this message has already circulated
    through.</li>

    <li> \c m_send indicates the cumulative minimum timestamp (aka
    Event::receiveTime) of all red events that have been dispatched by
    the various processes through which this message has already
    circulated.</li>

    <li> \c count is the list of vector counters that track the number
    of \i white messages that have been dispatched by various
    processes through which this message has already circulated.</li>
    
    </ul></a>
    
    </li>

    <a id="gvt_est_msg">
    <li> \c GVT_EST_MSG: This type of message (indicated by the \c
    kind member in this class) is used to circulate the estimated GVT
    value from the ROOT_KERNEL (process with rank 0) to all other
    processes. In this message only the m_clock value is used to
    indicate the current estimate of GVT.</li></a>

    <a id="gvt_ack_msg"> <li> \c GVT_ACK_MSG: This type of message
    (indicated by the \c kind member in this class) is used to report
    successful receipt of GVT update back to the ROOT_KERNEL (process
    with rank 0) from all other processes. In this message only the
    m_clock value is used to indicate the current estimate of
    GVT. This acknowledgement is necessary to ensure that the next
    cycle of GVT does not commence until the previous cycle is
    completed. </li></a>
    
    </ul>
*/
class GVTMessage {
    // Let the insertion operator be our friend
    friend std::ostream& ::operator << (std::ostream&, const muse::GVTMessage&);
public:
    /** \brief Enumeration for different kinds of GVT messages.

        This enumeration defines the different kinds of GVT messages
        that are circulated between various processes in the
        simulation.  The enumerations are interpreted as follows:

        <ul>

        <li> \c INVALID_GVT_MSG : This kind is not really used but
        provides a mechanism to tag messages as invalid messages if
        the need arises. </li>

        <li> \c GVT_CTRL_MSG : This kind identifies control messages
        that are circulated between processes.  See <a
        href="#gvt_ctrl_msg">earlier description</a> regarging this
        message for additional details.</li>

        <li> \c GVT_EST_MSG : This kind identifies messages that are
        circulated between processes to report estimated GVT value
        from ROOT_KERNEL (rank 0) to other processes.  See <a
        href="#gvt_est_msg">earlier description</a> regarging this
        message for additional details.</li>
        
        </ul>
    */
    enum GVTMsgKind{INVALID_GVT_MSG, GVT_CTRL_MSG, GVT_EST_MSG, GVT_ACK_MSG};

    /** \brief Method to create a GVT message.

        This method must be used to create a GVT message. The message
        is created such that memory is allocated in a serialized form
        and the message can be readily dispatched to another process.

        \param[in] kind The predefined kind of the message.

        \param[in] numProcesses If the message kind is \c GVT_CTRL_MSG
        then this parameter must have a non-zero value to indicate the
        number of processes in the simulation.  This information is
        used to allocate a vector to carry vector-counter information
        along with the message.
    */
    static GVTMessage* create(const GVTMsgKind kind,
                              const int numProcesses = 0);

    /** \brief Method to destroy (or delete) a GVT message.

        This method must be used to destroy (or delete) a GVT message
        thereby freeing the memory allocated to hold a GVT message.

        \param[in,out] msg The message to be destroyed. This pointer
        must have been obtained through a valid call to the create
        method in this class.  If this parameter is NULL, this method
        performs no specific operation.

        \note After this method call, the msg pointer is no longer
        points to a valid GVT message and its contents is undefined.
    */
    static void destroy(GVTMessage* msg);

    /** \brief Method to obtain the counter values associated with
        this message.

        This method must be used to obtain a pointer to the vector
        counter values stored in this class.  

        \return This method returns a pointer to (the first element)
        in vector counters stored in this message (if any).

        \note The returned pointer is valid only if the kind of this
        gvt message is \c GVT_CTRL_MSG.  The caller must not delete
        the returned pointer.
    */
    inline int* getCounters() { return count; }

    /** \brief Obtain the type of GVT message.

        This method must be used to determine what kind of GVT message
        (and associated information) that is contained by this GVT
        message.

        \return The kind value associated with this message.  This
        value is set when the message was created and is never changed
        during the life time of a GVTMessage object.
    */
    inline GVTMsgKind getKind() const { return kind; }

    /** \brief Obtain the raw size (in bytes) of this message.

        This method must be used to determine the raw size (in bytes)
        of this message.  This information is used to dispatch this
        message to another process via a suitable MPI call.

        \return The size of this message in bytes.  The returned value
        is set when the message was created and is never changed
        during the life time of a GVTMessage object.
    */
    inline int getSize() const { return size; }

    /** \brief Obtain the Tmin value associated with this event.

        This method must be used to obtain the cumulative minimum
        timestamp of all the outgoing events dispatched by the
        processes through which this message has circulated.
        
        \note This value is meaningful only in messages of kind \c
        GVT_CTRL_MSG.  In other messages, its value is undefined.

        \return The tMin value associated with this message. This
        value corresponds to the tMin value used in the pseudo code
        description of the algorithm in Mattern's paper.
    */
    inline Time getTmin() const { return tMin; }

    /** \brief Set the tMin value assocaited with this message.

        This method must be used to set the tMin value for this
        message.  This value is meaningful only in messages of kind \c
        GVT_CTRL_MSG.

        \param[in] tMin The tMin value to be set for this message.
        This value corresponds to the tMin value used in the pseudo
        code description of the algorithm in Mattern's paper.
    */
    void setTmin(const Time& tMin);
    
    /** \brief Obtain the current estimate of GVT.

        This method must be used to obtain the estimated GVT value
        stored in this message.  If the kind of the message is \c
        GVT_CTRL_MSG the returned value corresponds to the \c m_clock
        variable used in the pseudo code description of the algorithm
        in Mattern's paper.

        \return The currently estimated GVT value.
    */
    inline Time getGVTEstimate() const { return gvtEstimate; }

    /** \brief Set the estimated GVT value contained in this message.

        This method must be used to set the estimated GVT value
        contained in this message.

        \param[in] gvtEstimate The currently estimated GVT value to be
        stored in this message.
    */
    void setGVTEstimate(const Time& gvtEstimate);

    /** \brief Convenience method to obtain the minimum of GVT
        estimate and tMin to determine the actual GVT value.

        \return Returns min(gvtEstimate, tMin) which indicates the
        actual safe GVT estimate value.
    */
    inline Time getMin() const { return std::min<Time>(gvtEstimate, tMin); }
    
    /** \brief Utility method to determine if all counter values are
        zeros.

	This is a utility method that can be used to determine if all
	the counters in this event are all zeros.  This method is used
	in GVTManager::inspectRemoteEvent().

	\param[in] numProcesses The number of processes for which the
	counters must be checked.
	
	\return This method returns true if all the counters in this
	event are zeros.
    */
    bool areCountersZero(const int numProcesses) const;
    
 protected:
    /** \brief Constructor.

        The constructor has been made protected to ensure that this
        class is never directly instantiated.  Instead the static
        create() method must be used to create a GVT message.  The
        GVTMessage::create() method calls this constructor internally
        to create a GVT message.

        \param[in] kind The kind of GVT information contained in this
        message.  This value is simply copied to the corresponding
        instance variable in this class.

        \param[in] size The raw size (in bytes) of this GVT message.
        This value is simply copied to the corresponding instance
        variable in this class.
    */
    GVTMessage(const GVTMsgKind kind, const int size);

    /** \brief The destructor.

        The destructor has been made protected to ensure that a GVT
        message is never directly deleted.  It should not be directly
        deleted because it is not allocated as a standard object (but
        rather as a simple character array and must be correspondingly
        deleted).  Instead of invoking the destructor the
        GVTMessage::destory() method must be used for this task.
    */
    ~GVTMessage();

private:
    /** \brief Instance variable to hold the type of GVT message.

        This instance variable contains what kind of GVT message (and
        associated information) that is contained by this GVT
        message. This value is set by the constructor and is never
        changed during the life time of a GVTMessage object.
    */
    const GVTMsgKind kind;

    /** \brief Instance variable to hold the size of the message (in
        bytes).

        This method must be used to determine the raw size (in bytes)
        of this message.  This information is used to dispatch this
        message to another process via a suitable MPI call.  The size
        value is set by the constructor and is never changed during
        the life time of a GVTMessage object.
    */
    const int size;

    /** \brief The current GVT estimate value.

        This instance variable is used to hold the currently estimated
        GVT value associated with this message.  This value is set via
        the setGVTEstimate() method and accessed via the
        getGVTEstimate() method.

        \note This instance variable corresponds to the \c m_clock
        variable used in Mattern's paper to describe the GVT algorithm.
    */
    Time gvtEstimate;

    /** \brief The minimum timestamp of all outgoing events.

        This instance variable is used to the cumulative minimum
        timestamp of all the outgoing events dispatched by the
        processes through which this message has circulated.  This
        value is set via the setTmin() method and accessed via the
        getTmin() method.
        
        This value is meaningful only in messages of kind \c
        GVT_CTRL_MSG.  In other messages, its value is undefined.
    */
    Time tMin;

    /** \brief A unique sequence number set by Rank 0 process for this
        GVTMessage.

        This instance variable is used to track a global sequence number for
	each GVT token. The global sequence counter essentially helps to
	track GVT tokens as they circulate through the system and helps to
	troubleshoot issues. This value is set by the GVTManager::create
	method whenever a new GVTMessage is created.
    */
    unsigned int sequenceNumber;
    
    /** \brief The variable length array of vector counters.

        This instance variable contains a variable number of entries
        depending on the number of processes participating in a
        simulation.  This vector is valid only for messages of kind \c
        GVT_CTRL_MSG.  This vector is logically created when a
        suitable GVT message is instantiated and sufficient memory is
        allocated hold the data for this vector.
    */
    int count[];
};

END_NAMESPACE(muse);

#endif
